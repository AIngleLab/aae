/*

 */

using System;

namespace AIngle.Specific
{
    /// <summary>
    /// Base class for specific exceptions.
    /// </summary>
    public abstract class SpecificException : Exception, ISpecificRecord
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SpecificException"/> class.
        /// </summary>
        public SpecificException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SpecificException"/> class.
        /// </summary>
        /// <param name="message">
        /// The error message that explains the reason for the exception.
        /// </param>
        public SpecificException(string message) : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SpecificException"/> class.
        /// </summary>
        /// <param name="message">
        /// The error message that explains the reason for the exception.
        /// </param>
        /// <param name="innerException">
        /// The exception that is the cause of the current exception, or a null reference if no
        /// inner exception is specified.
        /// </param>
        public SpecificException(string message, Exception innerException) : base(message, innerException)
        {
        }

        /// <inheritdoc/>
        public abstract Schema Schema { get; }

        /// <inheritdoc/>
        public abstract object Get(int fieldPos);

        /// <inheritdoc/>
        public abstract void Put(int fieldPos, object fieldValue);
    }
}
