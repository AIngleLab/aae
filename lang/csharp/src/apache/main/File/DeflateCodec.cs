/*

 */
using System.IO;
using System.IO.Compression;

namespace AIngle.File
{
    /// <summary>
    /// Implements deflate compression and decompression.
    /// </summary>
    /// <seealso cref="DeflateStream"/>
    public class DeflateCodec : Codec
    {
        /// <inheritdoc/>
        public override byte[] Compress(byte[] uncompressedData)
        {
            MemoryStream outStream = new MemoryStream();

            using (DeflateStream Compress =
                        new DeflateStream(outStream,
                        CompressionMode.Compress))
            {
                Compress.Write(uncompressedData, 0, uncompressedData.Length);
            }
            return outStream.ToArray();
        }

        /// <inheritdoc/>
        public override byte[] Decompress(byte[] compressedData)
        {
            MemoryStream inStream = new MemoryStream(compressedData);
            MemoryStream outStream = new MemoryStream();

            using (DeflateStream Decompress =
                        new DeflateStream(inStream,
                        CompressionMode.Decompress))
            {
                CopyTo(Decompress, outStream);
            }
            return outStream.ToArray();
        }

        private static void CopyTo(Stream from, Stream to)
        {
            byte[] buffer = new byte[4096];
            int read;
            while((read = from.Read(buffer, 0, buffer.Length)) != 0)
            {
                to.Write(buffer, 0, read);
            }
        }

        /// <inheritdoc/>
        public override string GetName()
        {
            return DataFileConstants.DeflateCodec;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (this == other)
                return true;
            return this.GetType().Name == other.GetType().Name;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return DataFileConstants.DeflateCodecHash;
        }
    }
}
