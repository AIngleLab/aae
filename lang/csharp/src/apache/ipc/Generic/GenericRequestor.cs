/**

 */
using System;
using AIngle.Generic;
using AIngle.IO;

namespace AIngle.ipc.Generic
{
    public class GenericRequestor : Requestor
    {
        public GenericRequestor(Transceiver transceiver,Protocol protocol)
            : base(transceiver,protocol)
        {
        }

        public override void WriteRequest(RecordSchema schema, object request, Encoder encoder)
        {
            new GenericWriter<object>(schema).Write(request, encoder);
        }

        public override object ReadResponse(Schema writer, Schema reader, Decoder decoder)
        {
            return new GenericReader<Object>(writer, reader).Read(null, decoder);
        }

        public override Exception ReadError(Schema writer, Schema reader, Decoder decoder)
        {
            object results = new GenericReader<Object>(writer, reader).Read(null, decoder);

            return new Exception(results.ToString());
        }
    }
}
