/*

 */
using System;

namespace AIngle
{
    /// <summary>
    /// A generic AIngle exception.
    /// </summary>
    public class AIngleRuntimeException : AIngleException
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleRuntimeException"/> class.
        /// </summary>
        public AIngleRuntimeException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleRuntimeException"/> class.
        /// </summary>
        /// <param name="s">The message that describes the error.</param>
        public AIngleRuntimeException(string s)
            : base(s)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleRuntimeException"/> class.
        /// </summary>
        /// <param name="s">The message that describes the error.</param>
        /// <param name="inner">
        /// The exception that is the cause of the current exception, or a null reference
        /// if no inner exception is specified.
        /// </param>
        public AIngleRuntimeException(string s, Exception inner)
            : base(s, inner)
        {
        }
    }
}
