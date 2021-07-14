/*

 */
using System;

namespace AIngle
{
    /// <summary>
    /// Used to communicate an exception that occurred while parsing a protocol.
    /// </summary>
    public class ProtocolParseException : AIngleException
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ProtocolParseException"/> class.
        /// </summary>
        public ProtocolParseException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ProtocolParseException"/> class.
        /// </summary>
        /// <param name="s">Exception message.</param>
        public ProtocolParseException(string s)
            : base(s)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ProtocolParseException"/> class.
        /// </summary>
        /// <param name="s">Exception message.</param>
        /// <param name="inner">
        /// The exception that is the cause of the current exception, or a null reference
        /// if no inner exception is specified.
        /// </param>
        public ProtocolParseException(string s, Exception inner)
            : base(s, inner)
        {
        }
    }
}
