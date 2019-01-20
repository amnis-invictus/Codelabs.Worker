using System;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net;

namespace TestLib.Worker.Updater
{
	internal static class Program
	{
		private static Version getAssemblyVersion(string exePath)
		{
			var fvi = FileVersionInfo.GetVersionInfo(exePath);

			return new Version(fvi.ProductMajorPart, fvi.ProductMinorPart,
				fvi.ProductBuildPart, fvi.ProductPrivatePart);
		}

		[STAThread]
		private static void Main(string[] args)
		{
			//args[0] - current app
			//args[1] - pid
			//args[2] - Directory
			//args[3] - "TestLib.Worker.exe"

			if (args.Length != 4)
			{
				File.AppendAllText("log.txt", 
					string.Format("Count of arguments must be 4. Incorrect argument list."));
				return;
			}

			Process process = null;
			try
			{
				process = Process.GetProcessById(int.Parse(args[1]));
				process.WaitForExit();
			}
			catch (Exception ex)
			{
				File.AppendAllText("log.txt",
					string.Format("Error waiting for process {0}({1}) end: {2}",
					process?.ProcessName, process?.Id, ex));
				return;
			}

			WebClient client = new WebClient();

			string exeDir = args[2];
			string exePath = Path.Combine(exeDir, args[3]);

			Configuration config = ConfigurationManager.OpenExeConfiguration(exePath);
			string latestVersionUrl =
				config?.AppSettings?.Settings["update_latest_version_url"]?.Value
				?? "http://localhost:8081/api/version";
			string latestProgramUrl =
				config?.AppSettings?.Settings["update_latest_program_url"]?.Value
				?? "http://localhost:8081/api/update?version=latest";

			string stringVersion = client.DownloadString(latestVersionUrl).Replace("\"", "");
			if (!Version.TryParse(stringVersion, out var latestVersion))
			{
				File.AppendAllText("log.txt",
					string.Format("Can't parse latest version {0} from update server {1}. Update not available.",
					stringVersion, latestVersionUrl));

				return;
			}

			var currentVersion = getAssemblyVersion(exePath);
			bool need = latestVersion > currentVersion;

			var msg = string.Format("Latest version {0}, current version {1}: update is{2}necessary.",
				latestVersion, currentVersion, need ? " " : " not ");
			File.AppendAllText("log.txt", msg);

			if (!need)
			{
				return;
			}

			var tmpFile = Path.GetTempFileName();
			client.DownloadFile(latestProgramUrl, tmpFile);

			var archive = ZipFile.OpenRead(tmpFile);
			foreach (var entry in archive.Entries)
			{
				var path = Path.Combine(exeDir, entry.FullName);

				if (entry.FullName.EndsWith(".config", StringComparison.OrdinalIgnoreCase))
				{
					continue;
				}

				entry.ExtractToFile(path, true);
			}
		}
	}
}