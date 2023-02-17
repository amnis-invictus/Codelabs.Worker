using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace Worker.Helpers
{
	internal static class StringExtention
	{
		public static string ReplaceByDictionary(this string str, Regex re, Dictionary<string, string> replacement)
		{
			return re.Replace(str,
				match => replacement.TryGetValue(
					match.Value, out var value) ? value : match.Value);
		}
	}
}
