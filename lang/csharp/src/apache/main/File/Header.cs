/*

 */
using System.Collections.Generic;

namespace AIngle.File
{
    /// <summary>
    /// Header on an AIngle data file.
    /// </summary>
    public class Header
    {
        /// <summary>
        /// Metadata in this header.
        /// </summary>
        public IDictionary<string, byte[]> MetaData { get; }

        /// <summary>
        /// Sync token.
        /// </summary>
        public byte[] SyncData { get; }

        /// <summary>
        /// AIngle schema.
        /// </summary>
        public Schema Schema { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="Header"/> class.
        /// </summary>
        public Header()
        {
            MetaData = new Dictionary<string, byte[]>();
            SyncData = new byte[16];
        }
    }
}
