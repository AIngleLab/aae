/*

 */

using System;

namespace AIngle
{
    /// <summary>
    /// Used to communicate an exception that occurred while parsing a schema.
    /// </summary>
    public class SchemaParseException : AIngleException
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SchemaParseException"/> class.
        /// </summary>
        public SchemaParseException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SchemaParseException"/> class.
        /// </summary>
        /// <param name="s">Exception message.</param>
        public SchemaParseException(string s)
            : base(s)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SchemaParseException"/> class.
        /// </summary>
        /// <param name="message">
        /// The error message that explains the reason for the exception.
        /// </param>
        /// <param name="innerException">
        /// The exception that is the cause of the current exception, or a null reference if no
        /// inner exception is specified.
        /// </param>
        public SchemaParseException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
