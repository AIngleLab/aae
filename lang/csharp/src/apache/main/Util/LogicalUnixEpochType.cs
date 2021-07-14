/*

 */

using System;

namespace AIngle.Util
{
    /// <summary>
    /// Base for all logical type implementations that are based on the Unix Epoch date/time.
    /// </summary>
    public abstract class LogicalUnixEpochType<T> : LogicalType
        where T : struct
    {
        /// <summary>
        /// The date and time of the Unix Epoch.
        /// </summary>
        protected static readonly DateTime UnixEpochDateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

        /// <summary>
        /// Initializes the base logical type.
        /// </summary>
        /// <param name="name">The logical type name.</param>
        protected LogicalUnixEpochType(string name)
            : base(name)
        { }

        /// <inheritdoc/>
        public override Type GetCSharpType(bool nullible)
        {
            return nullible ? typeof(T?) : typeof(T);
        }

        /// <inheritdoc/>
        public override bool IsInstanceOfLogicalType(object logicalValue)
        {
            return logicalValue is T;
        }
    }
}
