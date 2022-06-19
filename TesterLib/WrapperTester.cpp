#include "Tester.h"
#include "Logger.h"
#include <vcclr.h>  
#include <msclr/gcroot.h>
#include <assert.h>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Runtime::CompilerServices;

namespace TesterLib
{
	class NativeLogCallbackHandler
	{
	public:
		NativeLogCallbackHandler(ref class LoggerManaged^ owner);
	private:
		static void OnLog(Internal::Logger & log, const uint8 level, const wchar_t * message, void * userData);
		msclr::gcroot<LoggerManaged^> owner;
	};

	public ref class LoggerManaged
	{
	public:
		enum class LogLevel : uint8
		{
			Trace,
			Debug,
			Info,
			Warn,
			Error,
			Fatal,
			Off,
		};

		delegate void LogEventHandler(ref class LoggerManaged^ log, LogLevel level, String^ message);

		LoggerManaged() :
			nativeLogger(new Internal::Logger()),
			nativeHandler(new NativeLogCallbackHandler(this))
		{ }

		~LoggerManaged()
		{
			this->!LoggerManaged();
		}

		!LoggerManaged()
		{
			delete nativeLogger;
			nativeLogger = nullptr;

			delete nativeHandler;
			nativeHandler = nullptr;
		}

		void Destroy()
		{
			delete nativeLogger;
			nativeLogger = nullptr;

			delete nativeHandler;
			nativeHandler = nullptr;
		}

		void InitNativeLogger(LogEventHandler ^ logger)
		{
			this->Log += logger;

			Internal::logger = this->GetNative();
			Internal::logger->Debug(L"Check native log from " __FUNCTIONW__);
		}

		event LogEventHandler^ Log
		{
			[MethodImplAttribute(MethodImplOptions::Synchronized)]
			void add(LogEventHandler^ value) {
				managedHandler = safe_cast<LogEventHandler^>(Delegate::Combine(value, managedHandler));
			}

			[MethodImplAttribute(MethodImplOptions::Synchronized)]
			void remove(LogEventHandler^ value) {
				managedHandler = safe_cast<LogEventHandler^>(Delegate::Remove(value, managedHandler));
			}

		private:
			void raise(LoggerManaged^ log, LogLevel level, String^ message)
			{
				if (managedHandler != nullptr)
					managedHandler(this, level, message);
			}
		}

	internal:
		void RaiseLogEvent(Byte level, String^ message)
		{
			Log(this, (LogLevel)level, message);
		}
		void RaiseLogEvent(LogLevel level, String^ message)
		{
			Log(this, level, message);
		}

		Internal::Logger * GetNative() { return nativeLogger; }

	private:
		Internal::Logger * nativeLogger;
		NativeLogCallbackHandler * nativeHandler;
		LogEventHandler^ managedHandler;
	};

	public ref class Tester
	{
	public:
		static Tester()
		{
			Internal::Tester::Init();
		}

		Tester()
		{
			tester = new Internal::Tester();
		}

		void SetProgram(String ^ program, String ^ args)
		{
			pin_ptr<const wchar_t> program_ptr = PtrToStringChars(program);
			pin_ptr<const wchar_t> args_ptr = PtrToStringChars(args);

			tester->SetProgram(program_ptr, args_ptr);
		}

		void SetUser(String ^ userName, String ^ domain, String ^ password)
		{
			pin_ptr<const wchar_t> userName_ptr = PtrToStringChars(userName);
			pin_ptr<const wchar_t> domain_ptr = PtrToStringChars(domain);
			pin_ptr<const wchar_t> password_ptr = PtrToStringChars(password);

			tester->SetUser(userName_ptr, domain_ptr, password_ptr);
		}

		void SetWorkDirectory(String ^ directory)
		{
			pin_ptr<const wchar_t> directory_ptr = PtrToStringChars(directory);

			tester->SetWorkDirectory(directory_ptr);
		}

		void SetRealTimeLimit(UInt32 timeMS)
		{
			tester->SetRealTimeLimit(timeMS);
		}

		void SetMemoryLimit(UInt32 memoryKB)
		{
			tester->SetMemoryLimit(memoryKB);
		}

		void RedirectIOHandleToFile(IOHandleType handleType, String ^ fileName)
		{
			if (fileName == nullptr)
				throw gcnew ArgumentNullException(nameof(fileName));

			using namespace Runtime::InteropServices;

			const wchar_t* fileName_native = (const wchar_t*)(Marshal::StringToHGlobalUni(fileName)).ToPointer();

			tester->RedirectIOHandleToFile(handleType, fileName_native);

			Marshal::FreeHGlobal(IntPtr((void*)fileName_native));
		}

		Boolean RedirectIOHandleToHandle(IOHandleType handleType, void * handle, bool duplicate)
		{
			if (handle == nullptr)
				throw gcnew ArgumentNullException(nameof(handle));

			return tester->RedirectIOHandleToHandle(handleType, handle, duplicate);
		}
		Boolean RedirectIOHandleToHandle(IOHandleType handleType, void * handle)
		{
			return RedirectIOHandleToHandle(handleType, handle, false);
		}
		Boolean RedirectIOHandleToHandle(IOHandleType handleType, IntPtr ^ handle, bool duplicate)
		{
			return RedirectIOHandleToHandle(handleType, handle->ToPointer(), duplicate);
		}
		Boolean RedirectIOHandleToHandle(IOHandleType handleType, IntPtr ^ handle)
		{
			return RedirectIOHandleToHandle(handleType, handle, false);
		}

		IntPtr ^ GetIORedirectedHandle(IOHandleType handleType)
		{
			return gcnew IntPtr(tester->GetIORedirectedHandle(handleType));
		}

		Boolean Run(Boolean useRestrictions)
		{
			return tester->Run(useRestrictions);
		}
		Boolean Run()
		{
			return tester->Run();
		}

		WaitingResult Wait()
		{
			return tester->Wait();
		}

		void CloseIORedirectionHandles()
		{
			tester->CloseIORedirectionHandles();
		}

		UInt32 GetRunResult()
		{
			return tester->GetRunResult();
		}
		UInt32 GetExitCode()
		{
			return tester->GetExitCode();
		}

		UsedResources GetUsedResources()
		{
			return tester->GetUsedResources();
		}

		void Destroy()
		{
			delete tester;
		}

	private:
		Internal::Tester * tester;

		static LoggerManaged ^ loggerManaged;
	};
	inline NativeLogCallbackHandler::NativeLogCallbackHandler(LoggerManaged ^ owner) : owner(owner)
	{
		owner->GetNative()->SetCallback(&OnLog, this);
	}
	inline void NativeLogCallbackHandler::OnLog(Internal::Logger & log, const uint8 level, const wchar_t * message, void * userData)
	{
		static_cast<NativeLogCallbackHandler*>(userData)->owner->RaiseLogEvent(level, gcnew String(message));
	}
}
