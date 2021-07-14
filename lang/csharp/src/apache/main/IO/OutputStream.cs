/*

 */

using System;
using System.IO;

namespace AIngle.IO
{
    /// <summary>
    /// Base class for an output stream.
    /// </summary>
    /// <seealso cref="InputStream"/>
    public abstract class OutputStream : Stream
    {
        /// <inheritdoc/>
        public override bool CanWrite
        {
            get { return true; }
        }

        /// <inheritdoc/>
        public override bool CanRead
        {
            get { return false; }
        }

        /// <inheritdoc/>
        public override bool CanSeek
        {
            get { return false; }
        }

        /// <inheritdoc/>
        public override long Position
        {
            get { throw new NotSupportedException(); }
            set { throw new NotSupportedException(); }
        }

        /// <inheritdoc/>
        public override int Read(byte[] buffer, int offset, int count)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc/>
        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc/>
        public override void SetLength(long value)
        {
            throw new NotSupportedException();
        }
    }
}
