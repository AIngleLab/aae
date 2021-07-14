/**

 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using AIngle.IO;

namespace AIngle.ipc
{
    public abstract class Transceiver
    {
        private readonly object channelLock = new object();
        private Thread threadWhenLocked;

        public virtual bool IsConnected
        {
            get { return false; }
        }

        public abstract String RemoteName { get; }

        public virtual Protocol Remote
        {
            get { throw new InvalidOperationException("Not connected."); }
            set { }
        }

        public virtual IList<MemoryStream> Transceive(IList<MemoryStream> request)
        {
            if (request == null) throw new ArgumentNullException("request");

            LockChannel();
            try
            {
                WriteBuffers(request);
                return ReadBuffers();
            }
            finally
            {
                UnlockChannel();
            }
        }

        public virtual void VerifyConnection()
        {
        }

        public void Transceive(IList<MemoryStream> request, ICallback<IList<MemoryStream>> callback)
        {
            if (request == null) throw new ArgumentNullException("request");

            try
            {
                IList<MemoryStream> response = Transceive(request);
                callback.HandleResult(response);
            }
            catch (IOException e)
            {
                callback.HandleException(e);
            }
        }

        public void LockChannel()
        {
            Monitor.Enter(channelLock);

            threadWhenLocked = Thread.CurrentThread;
        }

        public void UnlockChannel()
        {
            if (Thread.CurrentThread == threadWhenLocked)
            {
                Monitor.Exit(channelLock);
            }
        }

        public abstract IList<MemoryStream> ReadBuffers();
        public abstract void WriteBuffers(IList<MemoryStream> getBytes);
    }
}