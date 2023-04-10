using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml;
using System.Xml.Serialization;
using NLog;
using Worker.ClientApi;

namespace Worker
{
    internal class CompilerManager
    {
        static Logger logger = LogManager.GetCurrentClassLogger();

        private HashSet<byte> compilersIds = null;
        private Dictionary<byte, CompilerConfig> compilers = new Dictionary<byte, CompilerConfig>();
        private HttpCodelabsApiClient apiClient;

        public CompilerManager()
        {
            logger.Debug("ctor");
            apiClient = new HttpCodelabsApiClient();
        }
        public bool Init()
        {
            logger.Info("Initialization started.");
            IEnumerable<CompilerConfig> serverCompilers = apiClient.GetCompilers();
            if (!serverCompilers.Any())
            {
                logger.Error("Initialization failed, server does not provide any valid config.");
                return false;
            }

            serverCompilers = serverCompilers.Where(compilerInstalled);
            if (!serverCompilers.Any())
            {
                logger.Error("Initialization failed, server does not have installed compilers.");
                return false;
            }

            foreach (var compiler in serverCompilers)
            {
                compiler.Commands?.Sort((x, y) => x.Order - y.Order);
                compiler.InstallCompilerCommands?.Sort((x, y) => x.Order - y.Order);
                compilers.Add(compiler.Id, compiler);
            }

            logger.Info("Compiler's configuration initialized successfully.");
            return true;
        }

        public bool HasCompiler(byte id) =>
            compilers.ContainsKey(id);
        public bool CheckCompiler(byte id, string name) =>
            HasCompiler(id) && compilers[id].Name == name;
        public CompilerConfig GetCompiler(byte id) => compilers[id];
        public HashSet<byte> GetCompilers() =>
            compilersIds ?? (compilersIds = new HashSet<byte>(compilers.Keys));

        private bool compilerInstalled(CompilerConfig config) =>
            !string.IsNullOrEmpty(config.CheckCompilerFile) && File.Exists(config.CheckCompilerFile);
    }
}