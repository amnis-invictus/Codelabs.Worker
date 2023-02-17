using Newtonsoft.Json;
using Worker.Types;

namespace Worker.Models
{
	class Test
	{
        [JsonProperty(PropertyName = "id")]
        public ulong Id;
        [JsonProperty(PropertyName = "num")]
        public string Num;
        // Will be download from InputUrl
        public ProblemFile Input;
        // Will be download from AnswerUrl
        public ProblemFile Answer;

        [JsonProperty(PropertyName = "input_url")]
		public string InputUrl;
        [JsonProperty(PropertyName = "answer_arl")]
		public string AnswerUrl;
    }
}
