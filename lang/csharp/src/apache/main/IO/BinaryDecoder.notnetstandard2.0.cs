/*

 */
using System;
using System.Buffers;
using System.Buffers.Binary;
using System.Text;

namespace AIngle.IO
{
    /// <content>
    /// Contains the netstandard2.1 and netcoreapp2.1 specific functionality for BinaryDecoder.
    /// </content>
    public partial class BinaryDecoder
    {
        private const int StackallocThreshold = 256;

        /// <summary>
        /// A float is written as 4 bytes.
        /// The float is converted into a 32-bit integer using a method equivalent to
        /// Java's floatToIntBits and then encoded in little-endian format.
        /// </summary>
        /// <returns></returns>
        public float ReadFloat()
        {
            Span<byte> buffer = stackalloc byte[4];
            Read(buffer);

            return BitConverter.Int32BitsToSingle(BinaryPrimitives.ReadInt32LittleEndian(buffer));
        }

        /// <summary>
        /// A double is written as 8 bytes.
        /// The double is converted into a 64-bit integer using a method equivalent to
        /// Java's doubleToLongBits and then encoded in little-endian format.
        /// </summary>
        /// <returns>A double value.</returns>
        public double ReadDouble()
        {
            Span<byte> buffer = stackalloc byte[8];
            Read(buffer);

            return BitConverter.Int64BitsToDouble(BinaryPrimitives.ReadInt64LittleEndian(buffer));
        }

        /// <summary>
        /// Reads a string written by <see cref="BinaryEncoder.WriteString(string)"/>.
        /// </summary>
        /// <returns>String read from the stream.</returns>
        public string ReadString()
        {
            byte[] bufferArray = null;

            int length = ReadInt();
            Span<byte> buffer = length <= StackallocThreshold ?
                stackalloc byte[length] :
                (bufferArray = ArrayPool<byte>.Shared.Rent(length)).AsSpan(0, length);

            Read(buffer);

            string result = Encoding.UTF8.GetString(buffer);

            if (bufferArray != null)
            {
                ArrayPool<byte>.Shared.Return(bufferArray);
            }

            return result;
        }

        private void Read(byte[] buffer, int start, int len)
        {
            Read(buffer.AsSpan(start, len));
        }

        private void Read(Span<byte> buffer)
        {
            while (!buffer.IsEmpty)
            {
                int n = stream.Read(buffer);
                if (n <= 0) throw new AIngleException("End of stream reached");
                buffer = buffer.Slice(n);
            }
        }
    }
}
