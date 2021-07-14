/*

 */
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace AIngle.IO
{
    /// <summary>
    /// Utility to present <see cref="MemoryStream"/>s as an <see cref="InputStream"/>.
    /// </summary>
    /// <seealso cref="ByteBufferOutputStream"/>
    public class ByteBufferInputStream : InputStream
    {
        private readonly IList<MemoryStream> _buffers;
        private int _currentBuffer;

        /// <summary>
        /// Initializes a new instance of a <see cref="ByteBufferInputStream"/>.
        /// </summary>
        /// <param name="buffers"></param>
        public ByteBufferInputStream(IList<MemoryStream> buffers)
        {
            _buffers = buffers;
        }

        /// <inheritdoc/>
        public override int Read(byte[] b, int off, int len)
        {
            if (len == 0) return 0;
            MemoryStream buffer = GetNextNonEmptyBuffer();
            long remaining = buffer.Length - buffer.Position;
            if (len > remaining)
            {
                int remainingCheck = buffer.Read(b, off, (int) remaining);

                if(remainingCheck != remaining)
                    throw new InvalidDataException(string.Format(CultureInfo.InvariantCulture,
                        "remainingCheck [{0}] and remaining[{1}] are different.",
                        remainingCheck, remaining));
                return (int)remaining;
            }

            int lenCheck = buffer.Read(b, off, len);

            if (lenCheck != len)
                throw new InvalidDataException(string.Format(CultureInfo.InvariantCulture,
                    "lenCheck [{0}] and len[{1}] are different.", lenCheck, len));

            return len;
        }

        private MemoryStream GetNextNonEmptyBuffer()
        {
            while (_currentBuffer < _buffers.Count)
            {
                MemoryStream buffer = _buffers[_currentBuffer];
                if (buffer.Position < buffer.Length)
                    return buffer;

                _currentBuffer++;
            }
            throw new EndOfStreamException();
        }

        /// <summary>
        /// Throws a <see cref="NotSupportedException"/>.
        /// </summary>
        /// <exception cref="NotSupportedException">
        /// Always thows.
        /// </exception>
        public override long Length
        {
            get { throw new NotSupportedException(); }
        }
    }
}
