#include "Tester.h"

namespace Internal
{
	bool Tester::Run(bool useRestrictions)
	{
		Internal::logger->Debug(L"Called " __FUNCTIONW__ );

		if (!workDirSet || !programSet)
		{
			Internal::logger->Error(L"Can't start program w/o program name or workdirectory. workDirectory = '%s', program = '%s'", workDirectory, program);

			return false;
		}
		if (!realTimeLimitSet)
		{
			Internal::logger->Warning(L"Real time limit was not set. workDirectory = '%s', program = '%s'", workDirectory, program);
		}
		if (!memoryLimitSet)
		{
			Internal::logger->Warning(L"Memory limit was not set. workDirectory = '%s', program = '%s'", workDirectory, program);
		}

		HANDLE hProcessCreationToken = DuplicateCurrentProcessToken();
		startupHandles.job = CreateJobObjectW(nullptr, nullptr);

		STARTUPINFOEXW startupInfoEx = { 0 };
		startupInfoEx.StartupInfo.cb = sizeof(startupInfoEx);

		if (IoHandles.input != INVALID_HANDLE_VALUE)
		{
			startupInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
			startupInfoEx.StartupInfo.hStdInput = IoHandles.input;
		}
		if (IoHandles.output != INVALID_HANDLE_VALUE)
		{
			startupInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
			startupInfoEx.StartupInfo.hStdOutput = IoHandles.output;
		}
		if (IoHandles.error != INVALID_HANDLE_VALUE)
		{
			startupInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
			startupInfoEx.StartupInfo.hStdError = IoHandles.error;
		}

		if (useRestrictions)
		{
			applyMandatoryLevel(hProcessCreationToken);
			applyMemoryLimit();
			applyUIRestrictions();
			//applyStartupAttribute(&startupInfoEx);
		}

		PROCESS_INFORMATION processInfo = { 0 };
		BOOL result = CreateProcessAsUserW(hProcessCreationToken, program, args,
			nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT | CREATE_NEW_CONSOLE,
			nullptr, workDirectory, (STARTUPINFOW*)&startupInfoEx, &processInfo);

		if (!result)
		{
			Internal::logger->Error(L"Can't create process in " __FUNCTIONW__ " at line %d. CreateProcessAsUserW failed, error code %d", __LINE__, GetLastError());

			return false;
		}

		if (!AssignProcessToJobObject(startupHandles.job, processInfo.hProcess))
		{
			//log->LogErrorLastSystemError(L"in " __FUNCTION__);

			TerminateProcess(processInfo.hProcess, -1);
			SafeCloseHandle(&processInfo.hThread);
			SafeCloseHandle(&processInfo.hProcess);

			DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);
			free(startupInfoEx.lpAttributeList);
			TerminateJobObject(startupHandles.job, -1);

			SafeCloseHandle(&startupHandles.job);
			SafeCloseHandle(&hProcessCreationToken);

			return false;
		}

		BOOL res = FALSE;
		if (!IsProcessInJob(processInfo.hProcess, startupHandles.job, &res) || !res)
		{
			//log->LogErrorLastSystemError(L"in " __FUNCTION__);

			TerminateProcess(processInfo.hProcess, -1);
			SafeCloseHandle(&processInfo.hThread);
			SafeCloseHandle(&processInfo.hProcess);

			DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);
			free(startupInfoEx.lpAttributeList);
			TerminateJobObject(startupHandles.job, -1);

			SafeCloseHandle(&startupHandles.job);
			SafeCloseHandle(&hProcessCreationToken);
			return false;
		}

		startTime = GetTickCount();
		if (ResumeThread(processInfo.hThread) == (DWORD)-1)
		{
			//log->LogErrorLastSystemError(L"Can't resume main thread in " __FUNCTION__);

			TerminateProcess(processInfo.hProcess, -1);
			SafeCloseHandle(&processInfo.hThread);
			SafeCloseHandle(&processInfo.hProcess);

			DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);
			free(startupInfoEx.lpAttributeList);
			TerminateJobObject(startupHandles.job, -1);

			SafeCloseHandle(&startupHandles.job);
			SafeCloseHandle(&hProcessCreationToken);
			return false;
		}

		DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);
		free(startupInfoEx.lpAttributeList);
		SafeCloseHandle(&hProcessCreationToken);

		startupHandles.process = processInfo.hProcess;
		startupHandles.thread = processInfo.hThread;

		return true;
	}

	bool Tester::Wait()
	{
		DWORD timeOut = limits.realTimeLimitMs - (GetTickCount() - startTime);

		DWORD waitCode = WaitForSingleObject(startupHandles.process, timeOut);
		switch (waitCode)
		{
		case WAIT_TIMEOUT:
			TerminateProcess(startupHandles.process, -1);
			SafeCloseHandle(&startupHandles.thread);
			SafeCloseHandle(&startupHandles.process);

			TerminateJobObject(startupHandles.job, -1);
			SafeCloseHandle(&startupHandles.job);

			usedResources.realTimeUsageMs = limits.realTimeLimitMs + 1;
			usedResources.processExitCode = WAIT_TIMEOUT;

			Internal::logger->Error(L"Waiting program timeout expired. workDirectory = '%s', program = '%s'", workDirectory, program);
			break;
		case WAIT_FAILED:
			TerminateProcess(startupHandles.process, -1);
			SafeCloseHandle(&startupHandles.thread);
			SafeCloseHandle(&startupHandles.process);

			TerminateJobObject(startupHandles.job, -1);
			SafeCloseHandle(&startupHandles.job);

			usedResources.processExitCode = -1;

			Internal::logger->Error(L"Waiting program failed. workDirectory = '%s', program = '%s'", workDirectory, program);
			break;

		case WAIT_OBJECT_0:
			GetExitCodeProcess(startupHandles.process, &usedResources.processExitCode);
			Internal::logger->Info(L"Program waited successfully. workDirectory = '%s', program = '%s'", workDirectory, program);
			break;

		default:
			Internal::logger->Error(L"Error waiting process. Unknown status. status = %u, workDirectory = '%s', program = '%s'", waitCode, workDirectory, program);
			break;
		}

		usedResources.realTimeUsageMs = static_cast<uint32>(GetTickCount() - startTime);

		if (startupHandles.job != INVALID_HANDLE_VALUE)
		{
			JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicAccountingInfo;
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION exLimitInfo;

			if (!QueryInformationJobObject(startupHandles.job, JobObjectBasicAccountingInformation,
				&basicAccountingInfo, sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), nullptr))
			{
				//log->LogErrorLastSystemError(L"Error getting BASIC_ACCOUNTING_INFORMATION");
			}
			else
			{
				usedResources.cpuWorkTimeMs = static_cast<uint32>
					((basicAccountingInfo.TotalKernelTime.QuadPart + basicAccountingInfo.TotalUserTime.QuadPart) / 10000);
			}

			if (!QueryInformationJobObject(startupHandles.job, JobObjectExtendedLimitInformation,
				&exLimitInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION), nullptr))
			{
				//log->LogErrorLastSystemError(L"Error getting EXTENDED_LIMIT_INFORMATION");
			}
			else
			{
				usedResources.peakMemoryUsageKb = exLimitInfo.PeakJobMemoryUsed / 1024;
			}
		}

		return waitCode == WAIT_OBJECT_0;
	}
	void Tester::CloseIoRedirectionHandles()
	{
		if (IoHandles.input != INVALID_HANDLE_VALUE)
			FlushFileBuffers(IoHandles.input);
		if (IoHandles.output != INVALID_HANDLE_VALUE)
			FlushFileBuffers(IoHandles.output);
		if (IoHandles.error != INVALID_HANDLE_VALUE)
			FlushFileBuffers(IoHandles.error);

		SafeCloseHandle(&IoHandles.input);
		SafeCloseHandle(&IoHandles.output);
		SafeCloseHandle(&IoHandles.error);
	}
}
