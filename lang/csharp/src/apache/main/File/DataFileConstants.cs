/*

 */

namespace AIngle.File
{
    /// <summary>
    /// Constants used in data files.
    /// </summary>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Design",
        "CA1052:Static holder types should be Static or NotInheritable",
        Justification = "Maintain public API")]
    public class DataFileConstants
    {
        /// <summary>
        /// Key for the 'sync' metadata entry.
        /// </summary>
        public const string MetaDataSync = "aingle.sync";

        /// <summary>
        /// Key for the 'codec' metadata entry.
        /// </summary>
        public const string MetaDataCodec = "aingle.codec";

        /// <summary>
        /// Key for the 'schema' metadata entry.
        /// </summary>
        public const string MetaDataSchema = "aingle.schema";

        /// <summary>
        /// Identifier for the null codec.
        /// </summary>
        public const string NullCodec = "null";

        /// <summary>
        /// Identifier for the deflate codec.
        /// </summary>
        public const string DeflateCodec = "deflate";

        /// <summary>
        /// Reserved 'aingle' metadata key.
        /// </summary>
        public const string MetaDataReserved = "aingle";

        /// <summary>
        /// AIngle specification version.
        /// </summary>
        public const int Version = 1;

        /// <summary>
        /// Magic bytes at the beginning of an AIngle data file.
        /// </summary>
        public static byte[] Magic = { (byte)'O',
                                       (byte)'b',
                                       (byte)'j',
                                       Version };

        /// <summary>
        /// Hash code for the null codec.
        /// </summary>
        /// <seealso cref="NullCodec.GetHashCode()"/>
        public const int NullCodecHash = 2;

        /// <summary>
        /// Hash code for the deflate codec.
        /// </summary>
        /// <seealso cref="DeflateCodec.GetHashCode()"/>
        public const int DeflateCodecHash = 0;

        /// <summary>
        /// Size of a sync token in bytes.
        /// </summary>
        public const int SyncSize = 16;

        /// <summary>
        /// Default interval for sync tokens.
        /// </summary>
        public const int DefaultSyncInterval = 4000 * SyncSize;
    }
}
