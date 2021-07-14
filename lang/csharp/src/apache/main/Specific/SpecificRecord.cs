/*

 */

namespace AIngle.Specific
{
    /// <summary>
    /// Interface class for generated classes
    /// </summary>
    public interface ISpecificRecord
    {
        /// <summary>
        /// Schema of this instance.
        /// </summary>
        Schema Schema { get; }

        /// <summary>
        /// Return the value of a field given its position in the schema.
        /// This method is not meant to be called by user code, but only by
        /// <see cref="SpecificDatumReader{T}"/> implementations.
        /// </summary>
        /// <param name="fieldPos">Position of the field.</param>
        /// <returns>Value of the field.</returns>
        object Get(int fieldPos);

        /// <summary>
        /// Set the value of a field given its position in the schema.
        /// This method is not meant to be called by user code, but only by
        /// <see cref="SpecificDatumWriter{T}"/> implementations.
        /// </summary>
        /// <param name="fieldPos">Position of the field.</param>
        /// <param name="fieldValue">Value of the field.</param>
        void Put(int fieldPos, object fieldValue);
    }
}
