/*

 */

using System;

namespace AIngle.Util
{
    /// <summary>
    /// The 'timestamp-micros' logical type.
    /// </summary>
    public class TimestampMicrosecond : LogicalUnixEpochType<DateTime>
    {
        /// <summary>
        /// The logical type name for TimestampMicrosecond.
        /// </summary>
        public static readonly string LogicalTypeName = "timestamp-micros";

        /// <summary>
        /// Initializes a new TimestampMicrosecond logical type.
        /// </summary>
        public TimestampMicrosecond() : base(LogicalTypeName)
        { }

        /// <inheritdoc/>
        public override void ValidateSchema(LogicalSchema schema)
        {
            if (Schema.Type.Long != schema.BaseSchema.Tag)
                throw new AIngleTypeException("'timestamp-micros' can only be used with an underlying long type");
        }

        /// <inheritdoc/>
        public override object ConvertToBaseValue(object logicalValue, LogicalSchema schema)
        {
            var date = ((DateTime)logicalValue).ToUniversalTime();
            return (long)((date - UnixEpochDateTime).TotalMilliseconds * 1000);
        }

        /// <inheritdoc/>
        public override object ConvertToLogicalValue(object baseValue, LogicalSchema schema)
        {
            var noMs = (long)baseValue / 1000;
            return UnixEpochDateTime.AddMilliseconds(noMs);
        }
    }
}
