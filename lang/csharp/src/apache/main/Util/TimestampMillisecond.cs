/*

 */
using System;

namespace AIngle.Util
{
    /// <summary>
    /// The 'timestamp-millis' logical type.
    /// </summary>
    public class TimestampMillisecond : LogicalUnixEpochType<DateTime>
    {
        /// <summary>
        /// The logical type name for TimestampMillisecond.
        /// </summary>
        public static readonly string LogicalTypeName = "timestamp-millis";

        /// <summary>
        /// Initializes a new TimestampMillisecond logical type.
        /// </summary>
        public TimestampMillisecond() : base(LogicalTypeName)
        { }

        /// <inheritdoc/>
        public override void ValidateSchema(LogicalSchema schema)
        {
            if (Schema.Type.Long != schema.BaseSchema.Tag)
                throw new AIngleTypeException("'timestamp-millis' can only be used with an underlying long type");
        }

        /// <inheritdoc/>
        public override object ConvertToBaseValue(object logicalValue, LogicalSchema schema)
        {
            var date = ((DateTime)logicalValue).ToUniversalTime();
            return (long)(date - UnixEpochDateTime).TotalMilliseconds;
        }

        /// <inheritdoc/>
        public override object ConvertToLogicalValue(object baseValue, LogicalSchema schema)
        {
            var noMs = (long)baseValue;
            return UnixEpochDateTime.AddMilliseconds(noMs);
        }
    }
}
