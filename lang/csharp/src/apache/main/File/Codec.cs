/*

 */

namespace AIngle.File
{
    /// <summary>
    /// Base class for AIngle-supported compression codecs for data files. Note that Codec objects may
    /// maintain internal state (e.g. buffers) and are not thread safe.
    /// </summary>
    public abstract class Codec
    {
        /// <summary>
        /// Compress data using implemented codec
        /// </summary>
        /// <param name="uncompressedData"></param>
        /// <returns></returns>
        abstract public byte[] Compress(byte[] uncompressedData);

        /// <summary>
        /// Decompress data using implemented codec
        /// </summary>
        /// <param name="compressedData"></param>
        /// <returns></returns>
        abstract public byte[] Decompress(byte[] compressedData);

        /// <summary>
        /// Name of this codec type
        /// </summary>
        /// <returns></returns>
        abstract public string GetName();

        /// <summary>
        ///  Codecs must implement an equals() method
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        abstract public override bool Equals(object other);

        /// <summary>
        /// Codecs must implement a HashCode() method that is
        /// consistent with Equals
        /// </summary>
        /// <returns></returns>
        abstract public override int GetHashCode();

        /// <summary>
        /// Codec types
        /// </summary>
        public enum Type
        {
            /// <summary>
            /// Codec type that implments the "deflate" compression algorithm.
            /// </summary>
            Deflate,

            //Snappy

            /// <summary>
            /// Codec that does not perform any compression.
            /// </summary>
            Null
        };

        /// <summary>
        /// Factory method to return child
        /// codec instance based on Codec.Type
        /// </summary>
        /// <param name="codecType"></param>
        /// <returns></returns>
        public static Codec CreateCodec(Type codecType)
        {
            switch (codecType)
            {
                case Type.Deflate:
                    return new DeflateCodec();
                default:
                    return new NullCodec();
            }
        }

        /// <summary>
        /// Factory method to return child
        /// codec instance based on string type
        /// </summary>
        /// <param name="codecType"></param>
        /// <returns></returns>
        public static Codec CreateCodecFromString(string codecType)
        {
            switch (codecType)
            {
                case DataFileConstants.DeflateCodec:
                    return new DeflateCodec();
                default:
                    return new NullCodec();
            }
        }

        /// <summary>
        /// Returns name of codec
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return GetName();
        }
    }
}
