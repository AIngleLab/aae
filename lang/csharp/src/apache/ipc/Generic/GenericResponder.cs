/**

 */
using System;
using AIngle.Generic;
using Decoder = AIngle.IO.Decoder;
using Encoder = AIngle.IO.Encoder;

namespace AIngle.ipc.Generic
{
    public abstract class GenericResponder : Responder
    {
        protected GenericResponder(Protocol protocol)
            : base(protocol)
        {
        }

        static protected DatumWriter<Object> GetDatumWriter(Schema schema)
        {
            return new GenericWriter<Object>(schema);
        }

        static protected DatumReader<Object> GetDatumReader(Schema actual, Schema expected)
        {
            return new GenericReader<Object>(actual, expected);
        }

        public override object ReadRequest(Schema actual, Schema expected, Decoder input)
        {
            return GetDatumReader(actual, expected).Read(null, input);
        }

        public override void WriteResponse(Schema schema, object response, Encoder output)
        {
            GetDatumWriter(schema).Write(response, output);
        }
    }
}
