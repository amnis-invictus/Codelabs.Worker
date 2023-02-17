using Newtonsoft.Json;

namespace Worker.ClientApi.Models
{
    public class SubmissionLog
    {
        [JsonIgnore]
        public ulong SubmissionId;
        [JsonProperty(PropertyName = "type")]
        public SubmissionLogType Type;
        [JsonProperty(PropertyName = "data")]
        public string Data;
    }
}