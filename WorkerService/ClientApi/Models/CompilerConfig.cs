using Newtonsoft.Json;
using NLog;
using System.Collections.Generic;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using Worker.Types;

namespace Worker
{
    public class CompilerConfig
    {
        public byte Id { get; set; }
        public string FileExt { get; set; }
        public string Name { get; set; }
        public uint CompilersRealTimeLimit { get; set; }
        public List<Command> Commands { get; set; }
        public Command RunCommand { get; set; }
        public string CheckCompilerFile { get; set; }
        public Command CheckCompilerCommand { get; set; }
        public List<Command> InstallCompilerCommands { get; set; }

        static public CompilerConfig FromString(string data)
        {
            using (var readed = new StringReader(data))
                return FromTextReader(readed);
        }
        static public CompilerConfig FromFile(string file)
        {
            var info = new FileInfo(file);
            using (var stream = info.OpenRead())
                return FromStream(stream);
        }
        static public CompilerConfig FromStream(Stream stream)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(CompilerConfig));
            using (XmlReader reader = XmlReader.Create(stream))
                return (CompilerConfig)serializer.Deserialize(reader);
        }
        static public CompilerConfig FromTextReader(TextReader readed)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(CompilerConfig));
            using (XmlReader reader = XmlReader.Create(readed))
                return (CompilerConfig)serializer.Deserialize(reader);
        }
    }
}