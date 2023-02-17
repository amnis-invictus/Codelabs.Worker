namespace Worker.Helpers
{
    public static class StringConvertionExtension
	{
		public static int ToInt32OrDefault(this string value, int defaultValue = 0) =>
			 int.TryParse(value, out int result) ? result : defaultValue;

		public static uint ToUInt32OrDefault(this string value, uint defaultValue = 0) =>
			 uint.TryParse(value, out uint result) ? result : defaultValue;
	}
}
