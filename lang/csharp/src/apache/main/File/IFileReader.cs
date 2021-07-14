/*

 */
using System;
using System.Collections.Generic;

namespace AIngle.File
{
    /// <summary>
    /// Defines the interface for an object that reads data from a file.
    /// </summary>
    /// <typeparam name="T">Type to serialize data to.</typeparam>
    public interface IFileReader<T> : IDisposable
    {
        /// <summary>
        /// Return the header for the input file or stream.
        /// </summary>
        /// <returns>Parsed header from the file or stream.</returns>
        Header GetHeader();

        /// <summary>
        /// Return the schema as read from the file or stream.
        /// </summary>
        /// <returns>Parse schema from the file or stream.</returns>
        Schema GetSchema();

        /// <summary>
        /// Return the list of keys in the metadata.
        /// </summary>
        /// <returns>Metadata keys from the header of the data file.</returns>
        ICollection<string> GetMetaKeys();

        /// <summary>
        /// Return an enumeration of the remaining entries in the file.
        /// </summary>
        /// <returns>An enumeration of the remaining entries in the file.</returns>
        IEnumerable<T> NextEntries { get; }

        /// <summary>
        /// Read the next datum from the file.
        /// </summary>
        /// <returns>Next deserialized data entry.</returns>
        T Next();

        /// <summary>
        /// Returns true if more entries remain in this file.
        /// </summary>
        /// <returns>True if more entries remain in this file, false otherwise.</returns>
        bool HasNext();

        /// <summary>
        /// Return the byte value of a metadata property.
        /// </summary>
        /// <param name="key">Key for the metadata entry.</param>
        /// <returns>Raw bytes of the value of the metadata entry.</returns>
        /// <exception cref="KeyNotFoundException">
        /// There is no metadata entry with the specified <paramref name="key"/>.
        /// </exception>
        byte[] GetMeta(string key);

        /// <summary>
        /// Return the long value of a metadata property.
        /// </summary>
        /// <param name="key">Key for the metadata entry.</param>
        /// <returns>Metadata value as a long.</returns>
        /// <exception cref="KeyNotFoundException">
        /// There is no metadata entry with the specified <paramref name="key"/>.
        /// </exception>
        long GetMetaLong(string key);

        /// <summary>
        /// Return the string value of a metadata property. This method assumes that the string is a
        /// UTF-8 encoded in the header.
        /// </summary>
        /// <param name="key">Key for the metadata entry.</param>
        /// <returns>Metadata value as a string.</returns>
        /// <exception cref="KeyNotFoundException">
        /// There is no metadata entry with the specified <paramref name="key"/>.
        /// </exception>
        /// <exception cref="AIngleRuntimeException">
        /// Encountered an exception while decoding the value as a UTF-8 string.
        /// </exception>
        string GetMetaString(string key);

        /// <summary>
        /// Return true if past the next synchronization point after a position.
        /// </summary>
        /// <param name="position">Position to test.</param>
        /// <returns>
        /// True if pasth the next synchronization point after <paramref name="position"/>, false
        /// otherwise.
        /// </returns>
        bool PastSync(long position);

        /// <summary>
        /// Return the last synchronization point before our current position.
        /// </summary>
        /// <returns>
        /// Position of the last synchronization point before our current position.
        /// </returns>
        long PreviousSync();

        /// <summary>
        /// Move to a specific, known synchronization point,
        /// one returned from <see cref="IFileWriter{T}.Sync"/> while writing.
        /// </summary>
        /// <param name="position">Position to jump to.</param>
        void Seek(long position);

        /// <summary>
        /// Move to the next synchronization point after a position.
        /// </summary>
        /// <param name="position">Position in the stream to start.</param>
        void Sync(long position);

        /// <summary>
        /// Return the current position in the input.
        /// </summary>
        /// <returns>Current position in the input.</returns>
        long Tell();
    }
}
