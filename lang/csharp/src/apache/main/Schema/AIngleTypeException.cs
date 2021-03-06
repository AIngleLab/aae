/*

 */

using System;

namespace AIngle
{
    /// <summary>
    /// Used to communicate an exception associated with AIngle typing.
    /// </summary>
    public class AIngleTypeException : AIngleException
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleTypeException"/> class.
        /// </summary>
        public AIngleTypeException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleTypeException"/> class.
        /// </summary>
        /// <param name="s"></param>
        public AIngleTypeException(string s)
            : base(s)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleTypeException"/> class.
        /// </summary>
        /// <param name="message">
        /// The error message that explains the reason for the exception.
        /// </param>
        /// <param name="innerException">
        /// The exception that is the cause of the current exception, or a null reference if no
        /// inner exception is specified.
        /// </param>
        public AIngleTypeException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
