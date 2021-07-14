/*

 */
using AIngle.IO;

namespace AIngle.Generic
{
    /// <summary>
    /// Defines the interface for an object that writes data of a schema.
    /// </summary>
    /// <typeparam name="T">Type of the in-memory data representation.</typeparam>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Naming",
        "CA1715:Identifiers should have correct prefix", Justification = "Maintain public API")]
    public interface DatumWriter<T>
    {
        /// <summary>
        /// Schema used to write the data.
        /// </summary>
        Schema Schema { get; }

        /// <summary>
        /// Write a datum. Traverse the schema, depth first, writing each leaf value in the schema
        /// from the datum to the output.
        /// </summary>
        /// <param name="datum">Datum to write</param>
        /// <param name="encoder">Encoder to write to</param>
        void Write(T datum, Encoder encoder);
    }
}
