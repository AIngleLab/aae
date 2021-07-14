/**

 */

using System;
using AIngle.Generic;
using AIngle.IO;
using AIngle.ipc;
using AIngle.ipc.Generic;
using NUnit.Framework;

namespace AIngle.Test.Ipc
{
    [TestFixture]
    public class LocalTransceiverTest
    {
        [TestCase]
        public void TestSingleRpc()
        {
            Transceiver t = new LocalTransceiver(new TestResponder(protocol));
            var p = new GenericRecord(protocol.Messages["m"].Request);
            p.Add("x", "hello");
            var r = new GenericRequestor(t, protocol);

            for (int x = 0; x < 5; x++)
            {
                object request = r.Request("m", p);
                Assert.AreEqual("there", request);
            }
        }


        private readonly Protocol protocol = Protocol.Parse("" + "{\"protocol\": \"Minimal\", "
                                                            + "\"messages\": { \"m\": {"
                                                            +
                                                            "   \"request\": [{\"name\": \"x\", \"type\": \"string\"}], "
                                                            + "   \"response\": \"string\"} } }");

        public class TestResponder : GenericResponder
        {
            public TestResponder(Protocol local)
                : base(local)
            {
            }

            public override object Respond(Message message, object request)
            {
                Assert.AreEqual("hello", ((GenericRecord) request)["x"]);
                return "there";
            }

            public override void WriteError(Schema schema, object error, Encoder output)
            {
                throw new NotSupportedException();
            }
        }
    }
}