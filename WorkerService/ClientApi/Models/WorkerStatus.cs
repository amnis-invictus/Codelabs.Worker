namespace Worker.ClientApi.Models
{
    internal enum WorkerStatus : byte
	{
		Disabled = 0,
		Ok = 1,
		Failed = 2,
		Stale = 3,
		Stopped = 4,
	}
}
