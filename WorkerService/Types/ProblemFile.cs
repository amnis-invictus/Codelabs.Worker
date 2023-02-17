namespace Worker.Types
{
	class ProblemFile
	{
		public string id;
		public byte[] Content;

		public void Save()
		{
			Application.Get().FileProvider.SaveFile(this);
        }
	}

	/*class FileDb
	{
		public string id;
		public string Name;
	}*/
}
