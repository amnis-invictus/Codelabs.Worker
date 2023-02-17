namespace Worker.ClientApi.Models
{
    public enum WorkerResult : byte
    {
        Ok,
        CompilerError,
        TestingError,
    }
}