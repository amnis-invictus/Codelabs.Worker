﻿using System;
using System.Collections.Generic;
using Worker.ClientApi.Models;
using Worker.Models;
using Worker.Types;

namespace Worker.ClientApi
{
	interface IApiClient
	{
		Guid SignUp(WorkerInformation worker);
		UpdateWorkerStatus SignIn(Guid id);
		bool SignOut(Guid id);
		UpdateWorkerStatus UpdateWorker(Guid id, WorkerInformation worker);

		Problem DownloadProblem(ulong problemId);
		ProblemFile DownloadSolution(Submission submission);
		IEnumerable<Submission> GetSubmissions(bool retryOnFailed = true);
		IEnumerable<CompilerConfig> GetCompilers();

		RequestMessage GetTakeSubmissionsRequestMessage(ulong id);
		RequestMessage GetFailSubmissionsRequestMessage(ulong id);
		RequestMessage GetReleaseSubmissionsRequestMessage(ulong id, WorkerResult result);
		RequestMessage GetSendTestingResultRequestMessage(TestResult result);
		RequestMessage GetSendLogRequestMessage(SubmissionLog log);

		bool SendRequest(RequestMessage message, bool retryOnFailed = true);

		uint GetVersion();
		ApiType GetApiType();
		bool GetWebhookSupported();
	}
}