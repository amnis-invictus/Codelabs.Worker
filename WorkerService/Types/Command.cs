namespace Worker.Types
{
    public class Command
    {
        public int Order { get; set; }
        public string Program { get; set; }
        public string Arguments { get; set; }
        public string CheckerArguments { get; set; }
	}
}
