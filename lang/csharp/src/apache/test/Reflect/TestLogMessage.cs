/**

 */

using System;
using System.Collections.Generic;
using System.IO;
using AIngle.IO;
using AIngle.Reflect;
using NUnit.Framework;

namespace AIngle.Test
{
    public enum MessageTypes
    {
        None,
        Verbose,
        Info,
        Warning,
        Error
    }

    public class LogMessage
    {
        private Dictionary<string, string> _tags = new Dictionary<string, string>();

        public string IP { get; set; }

        [AIngleField("Message")]
        public string message { get; set; }

        [AIngleField(typeof(DateTimeOffsetToLongConverter))]
        public DateTimeOffset TimeStamp { get; set; }

        public Dictionary<string, string> Tags { get => _tags; set => _tags = value; }

        public MessageTypes Severity { get; set; }
    }

    [TestFixture]
    public class TestLogMessage
    {
        private const string _logMessageSchemaV1 = @"
        {
            ""namespace"": ""MessageTypes"",
            ""type"": ""record"",
            ""doc"": ""A simple log message type as used by this blog post."",
            ""name"": ""LogMessage"",
            ""fields"": [
                { ""name"": ""IP"", ""type"": ""string"" },
                { ""name"": ""Message"", ""type"": ""string"" },
                { ""name"": ""TimeStamp"", ""type"": ""long"" },
                { ""name"": ""Tags"",""type"":
                    { ""type"": ""map"",
                        ""values"": ""string""},
                        ""default"": {}},
                { ""name"": ""Severity"",
                ""type"": { ""namespace"": ""MessageTypes"",
                    ""type"": ""enum"",
                    ""doc"": ""Enumerates the set of allowable log levels."",
                    ""name"": ""LogLevel"",
                    ""symbols"": [""None"", ""Verbose"", ""Info"", ""Warning"", ""Error""]}}
            ]
        }";

        [TestCase]
        public void Serialize()
        {
            var schema = Schema.Parse(_logMessageSchemaV1);
            var aingleWriter = new ReflectWriter<LogMessage>(schema);
            var aingleReader = new ReflectReader<LogMessage>(schema, schema);

            byte[] serialized;

            var logMessage = new LogMessage()
            {
                IP = "10.20.30.40",
                message = "Log entry",
                Severity = MessageTypes.Error
            };

            using (var stream = new MemoryStream(256))
            {
                aingleWriter.Write(logMessage, new BinaryEncoder(stream));
                serialized = stream.ToArray();
            }

            LogMessage deserialized = null;
            using (var stream = new MemoryStream(serialized))
            {
                deserialized = aingleReader.Read(default(LogMessage), new BinaryDecoder(stream));
            }
            Assert.IsNotNull(deserialized);
            Assert.AreEqual(logMessage.IP, deserialized.IP);
            Assert.AreEqual(logMessage.message, deserialized.message);
            Assert.AreEqual(logMessage.Severity, deserialized.Severity);
        }
    }
}
