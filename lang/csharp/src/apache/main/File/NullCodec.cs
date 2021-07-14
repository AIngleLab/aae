/*

 */

namespace AIngle.File
{
    /// <summary>
    /// Implements a codec that does not perform any compression. This codec simply returns the
    /// bytes presented to it "as-is".
    /// </summary>
    public class NullCodec : Codec
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="NullCodec"/> class.
        /// </summary>
        public NullCodec() { }

        /// <inheritdoc/>
        public override byte[] Compress(byte[] uncompressedData)
        {
            return uncompressedData;
        }

        /// <inheritdoc/>
        public override byte[] Decompress(byte[] compressedData)
        {
            return compressedData;
        }

        /// <inheritdoc/>
        public override string GetName()
        {
            return DataFileConstants.NullCodec;
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
            return DataFileConstants.NullCodecHash;
        }
    }
}
