/*

 */
using System;
using System.IO;

namespace AIngle.IO
{
    /// <summary>
    /// Write leaf values.
    /// </summary>
    public class BinaryEncoder : Encoder
    {
        private readonly Stream Stream;

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryEncoder"/> class without a backing
        /// stream.
        /// </summary>
        public BinaryEncoder() : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryEncoder"/> class that writes to
        /// the provided stream.
        /// </summary>
        /// <param name="stream">Stream to write to.</param>
        public BinaryEncoder(Stream stream)
        {
            this.Stream = stream;
        }

        /// <summary>
        /// null is written as zero bytes
        /// </summary>
        public void WriteNull()
        {
        }

        /// <summary>
        /// true is written as 1 and false 0.
        /// </summary>
        /// <param name="b">Boolean value to write</param>
        public void WriteBoolean(bool b)
        {
            writeByte((byte)(b ? 1 : 0));
        }

        /// <summary>
        /// int and long values are written using variable-length, zig-zag coding.
        /// </summary>
        /// <param name="value">Value to write</param>
        public void WriteInt(int value)
        {
            WriteLong(value);
        }
        /// <summary>
        /// int and long values are written using variable-length, zig-zag coding.
        /// </summary>
        /// <param name="value">Value to write</param>
        public void WriteLong(long value)
        {
            ulong n = (ulong)((value << 1) ^ (value >> 63));
            while ((n & ~0x7FUL) != 0)
            {
                writeByte((byte)((n & 0x7f) | 0x80));
                n >>= 7;
            }
            writeByte((byte)n);
        }

        /// <summary>
        /// A float is written as 4 bytes.
        /// The float is converted into a 32-bit integer using a method equivalent to
        /// Java's floatToIntBits and then encoded in little-endian format.
        /// </summary>
        /// <param name="value"></param>
        public void WriteFloat(float value)
        {
            byte[] buffer = BitConverter.GetBytes(value);
            if (!BitConverter.IsLittleEndian) Array.Reverse(buffer);
            writeBytes(buffer);
        }
        /// <summary>
        ///A double is written as 8 bytes.
        ///The double is converted into a 64-bit integer using a method equivalent to
        ///Java's doubleToLongBits and then encoded in little-endian format.
        /// </summary>
        /// <param name="value"></param>
        public void WriteDouble(double value)
        {
            long bits = BitConverter.DoubleToInt64Bits(value);

            writeByte((byte)(bits & 0xFF));
            writeByte((byte)((bits >> 8) & 0xFF));
            writeByte((byte)((bits >> 16) & 0xFF));
            writeByte((byte)((bits >> 24) & 0xFF));
            writeByte((byte)((bits >> 32) & 0xFF));
            writeByte((byte)((bits >> 40) & 0xFF));
            writeByte((byte)((bits >> 48) & 0xFF));
            writeByte((byte)((bits >> 56) & 0xFF));

        }

        /// <summary>
        /// Bytes are encoded as a long followed by that many bytes of data.
        /// </summary>
        /// <param name="value"></param>
        public void WriteBytes(byte[] value)
        {
            WriteLong(value.Length);
            writeBytes(value);
        }

        /// <summary>
        /// A string is encoded as a long followed by
        /// that many bytes of UTF-8 encoded character data.
        /// </summary>
        /// <param name="value"></param>
        public void WriteString(string value)
        {
            WriteBytes(System.Text.Encoding.UTF8.GetBytes(value));
        }

        /// <inheritdoc/>
        public void WriteEnum(int value)
        {
            WriteLong(value);
        }

        /// <inheritdoc/>
        public void StartItem()
        {
        }

        /// <inheritdoc/>
        public void SetItemCount(long value)
        {
            if (value > 0) WriteLong(value);
        }

        /// <inheritdoc/>
        public void WriteArrayStart()
        {
        }

        /// <inheritdoc/>
        public void WriteArrayEnd()
        {
            WriteLong(0);
        }

        /// <inheritdoc/>
        public void WriteMapStart()
        {
        }

        /// <inheritdoc/>
        public void WriteMapEnd()
        {
            WriteLong(0);
        }

        /// <inheritdoc/>
        public void WriteUnionIndex(int value)
        {
            WriteLong(value);
        }

        /// <inheritdoc/>
        public void WriteFixed(byte[] data)
        {
            WriteFixed(data, 0, data.Length);
        }

        /// <inheritdoc/>
        public void WriteFixed(byte[] data, int start, int len)
        {
            Stream.Write(data, start, len);
        }

        private void writeBytes(byte[] bytes)
        {
            Stream.Write(bytes, 0, bytes.Length);
        }

        private void writeByte(byte b)
        {
            Stream.WriteByte(b);
        }

        /// <summary>
        /// Flushes the underlying stream.
        /// </summary>
        public void Flush()
        {
            Stream.Flush();
        }
    }
}
