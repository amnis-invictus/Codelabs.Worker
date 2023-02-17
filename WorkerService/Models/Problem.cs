using Newtonsoft.Json;
using System;
using Worker.Types;

namespace Worker.Models
{
    class Problem
    {
        [JsonProperty(PropertyName = "id")]
        public ulong Id { get; private set; }
        [JsonProperty(PropertyName = "tests")]
        public Test[] Tests { get; private set; }
        [JsonProperty(PropertyName = "checker_compiler_id")]
        public byte CheckerCompilerId { get; private set; }
        // Will be download from CheckerSourceUrl
        public ProblemFile Checker { get; set; }
        [JsonProperty(PropertyName = "updated_at")]
        public DateTime LastUpdate { get; private set; }
        /*
         * Used by Newtonsoft.Json.JsonConvert.DeserializeObject<Problem>
         * It create object without checker, because it should be downloaded
         */
        public Problem() { }
        public Problem(ulong id, Test[] tests, ProblemFile checker, byte checkerCompilerId, DateTime lastUpdate)
        {
            Id = id;
            Tests = tests ?? throw new ArgumentNullException(nameof(tests));
            Checker = checker ?? throw new ArgumentNullException(nameof(checker));
            CheckerCompilerId = checkerCompilerId;
            LastUpdate = lastUpdate;
        }

        [JsonProperty(PropertyName = "checker_source_url")]
        public string CheckerSourceUrl { get; private set; }
    }
}
