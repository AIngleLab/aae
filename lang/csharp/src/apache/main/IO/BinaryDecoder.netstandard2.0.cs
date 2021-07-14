/*

 */
using System;

namespace AIngle.IO
{
    /// <content>
    /// Contains the netstandard2.0 specific functionality for BinaryDecoder.
    /// </content>
    public partial class BinaryDecoder
    {
        /// <summary>
        /// A float is written as 4 bytes.
        /// The float is converted into a 32-bit integer using a method equivalent to
        /// Java's floatToIntBits and then encoded in little-endian format.
        /// </summary>
        /// <returns></returns>
        public float ReadFloat()
        {
            byte[] buffer = read(4);

            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer);

            return BitConverter.ToSingle(buffer, 0);

            //int bits = (Stream.ReadByte() & 0xff |
            //(Stream.ReadByte()) & 0xff << 8 |
            //(Stream.ReadByte()) & 0xff << 16 |
            //(Stream.ReadByte()) & 0xff << 24);
            //return intBitsToFloat(bits);
        }

        /// <summary>
        /// A double is written as 8 bytes.
        /// The double is converted into a 64-bit integer using a method equivalent to
        /// Java's doubleToLongBits and then encoded in little-endian format.
        /// </summary>
        /// <returns>A double value.</returns>
        public double ReadDouble()
        {
            long bits = (stream.ReadByte() & 0xffL) |
              (stream.ReadByte() & 0xffL) << 8 |
              (stream.ReadByte() & 0xffL) << 16 |
              (stream.ReadByte() & 0xffL) << 24 |
              (stream.ReadByte() & 0xffL) << 32 |
              (stream.ReadByte() & 0xffL) << 40 |
              (stream.ReadByte() & 0xffL) << 48 |
              (stream.ReadByte() & 0xffL) << 56;
            return BitConverter.Int64BitsToDouble(bits);
        }

        /// <summary>
        /// Reads a string written by <see cref="BinaryEncoder.WriteString(string)"/>.
        /// </summary>
        /// <returns>String read from the stream.</returns>
        public string ReadString()
        {
            int length = ReadInt();
            byte[] buffer = new byte[length];
            //TODO: Fix this because it's lame;
            ReadFixed(buffer);
            return System.Text.Encoding.UTF8.GetString(buffer);
        }

        private void Read(byte[] buffer, int start, int len)
        {
            while (len > 0)
            {
                int n = stream.Read(buffer, start, len);
                if (n <= 0) throw new AIngleException("End of stream reached");
                start += n;
                len -= n;
            }
        }
    }
}
