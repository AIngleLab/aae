/**

 */
using System;
using System.Collections.Generic;
using System.IO;

namespace AIngle.ipc
{
    public class LocalTransceiver : Transceiver
    {
        private readonly Responder responder;

        public LocalTransceiver(Responder responder)
        {
            if (responder == null) throw new ArgumentNullException("responder");

            this.responder = responder;
        }

        public override string RemoteName
        {
            get { return "local"; }
        }

        public override IList<MemoryStream> Transceive(IList<MemoryStream> request)
        {
            return responder.Respond(request);
        }

        public override IList<MemoryStream> ReadBuffers()
        {
            throw new NotSupportedException();
        }

        public override void WriteBuffers(IList<MemoryStream> getBytes)
        {
            throw new NotSupportedException();
        }
    }
}