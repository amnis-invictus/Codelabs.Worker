using System;
using Worker.Types;

namespace Worker.Models
{
	class Problem
	{
		public ulong Id { get; private set; }
		public Test[] Tests { get; private set; }
		public byte CheckerCompilerId { get; private set; }
		public ProblemFile Checker { get; private set; }
		public DateTime LastUpdate { get; private set; }

		public Problem(ulong id, Test[] tests, ProblemFile checker, byte checkerCompilerId, DateTime lastUpdate)
		{
			Id = id;
			Tests = tests ?? throw new ArgumentNullException(nameof(tests));
			Checker = checker ?? throw new ArgumentNullException(nameof(checker));
			CheckerCompilerId = checkerCompilerId;
			LastUpdate = lastUpdate;
		}
	}
}
