/*

 */
using System;
using System.IO;

namespace AIngle.File
{
    /// <summary>
    /// Encapsulates a block of data read by the <see cref="DataFileReader{T}"/>.
    /// We will remove this class from the public API in a future version because it is only meant
    /// to be used internally.
    /// </summary>
    [Obsolete("This will be removed from the public API in a future version.")]
    public class DataBlock
    {
        /// <summary>
        /// Raw bytes within this block.
        /// </summary>
        public byte[] Data { get;  set; }

        /// <summary>
        /// Number of entries in this block.
        /// </summary>
        public long NumberOfEntries { get; set; }

        /// <summary>
        /// Size of this block in bytes.
        /// </summary>
        public long BlockSize { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="DataBlock"/> class.
        /// </summary>
        /// <param name="numberOfEntries">Number of entries in this block.</param>
        /// <param name="blockSize">Size of this block in bytes.</param>
        public DataBlock(long numberOfEntries, long blockSize)
        {
            NumberOfEntries = numberOfEntries;
            BlockSize = blockSize;
            Data = new byte[blockSize];
        }

        internal Stream GetDataAsStream()
        {
            return new MemoryStream(Data);
        }
    }
}
