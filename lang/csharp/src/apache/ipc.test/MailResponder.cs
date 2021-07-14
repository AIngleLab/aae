/**

 */

using System;
using System.IO;
using System.Reflection;
using AIngle.Generic;
using AIngle.IO;
using AIngle.ipc;
using AIngle.ipc.Generic;
using NUnit.Framework;

namespace AIngle.Test.Ipc
{
    public class MailResponder : GenericResponder
    {
        private static Protocol protocol;
        private CountdownLatch allMessages = new CountdownLatch(5);

        public MailResponder()
            : base(Protocol)
        {
        }

        public static Protocol Protocol
        {
            get
            {
                if (protocol == null)
                {
                    string readAllLines;
                    using (
                        Stream stream =
                            Assembly.GetExecutingAssembly().GetManifestResourceStream("AIngle.ipc.test.mail.avpr"))
                    using (var reader = new StreamReader(stream))
                    {
                        readAllLines = reader.ReadToEnd();
                    }

                    protocol = Protocol.Parse(readAllLines);
                }

                return protocol;
            }
        }

        public override object Respond(Message message, object request)
        {
            if (message.Name == "send")
            {
                var genericRecord = (GenericRecord) ((GenericRecord) request)["message"];

                return "Sent message to [" + genericRecord["to"] +
                       "] from [" + genericRecord["from"] + "] with body [" +
                       genericRecord["body"] + "]";
            }
            if (message.Name == "fireandforget")
            {
                allMessages.Signal();
                return null;
            }

            throw new NotSupportedException();
        }

        public void Reset()
        {
            allMessages = new CountdownLatch(5);
        }

        public void AwaitMessages()
        {
            allMessages.Wait(2000);
        }

        public void AssertAllMessagesReceived()
        {
            Assert.AreEqual(0, allMessages.CurrentCount);
        }


        public override void WriteError(Schema schema, object error, Encoder output)
        {
            Assert.Fail(error.ToString());
        }
    }
}
