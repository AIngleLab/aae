/**

 */

using System;
using System.Threading;

namespace AIngle.ipc
{
    public class CountdownLatch : IDisposable
    {
        private ManualResetEvent evt;
        private int currentCount;

        public int CurrentCount
        {
            get { return currentCount; }
        }

        public CountdownLatch(int count)
        {
            currentCount = count;
            evt = new ManualResetEvent(false);
        }

        public void Signal()
        {
            if (Interlocked.Decrement(ref currentCount) == 0)
                evt.Set();
        }

        public void Wait()
        {
            evt.WaitOne();
        }

        public bool Wait(int milliseconds)
        {
            return evt.WaitOne(milliseconds);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ~CountdownLatch()
        {
            // Finalizer calls Dispose(false)
            Dispose(false);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                // free managed resources
                if (evt != null)
                {
                    evt.Close();
                    evt = null;
                }
            }
        }
    }
}