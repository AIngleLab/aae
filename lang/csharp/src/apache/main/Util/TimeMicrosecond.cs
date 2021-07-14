/*

 */

using System;

namespace AIngle.Util
{
    /// <summary>
    /// The 'time-micros' logical type.
    /// </summary>
    public class TimeMicrosecond : LogicalUnixEpochType<TimeSpan>
    {
        private static readonly TimeSpan _maxTime = new TimeSpan(23, 59, 59);
        
        /// <summary>
        /// The logical type name for TimeMicrosecond.
        /// </summary>
        public static readonly string LogicalTypeName = "time-micros";

        /// <summary>
        /// Initializes a new TimeMicrosecond logical type.
        /// </summary>
        public TimeMicrosecond() : base(LogicalTypeName)
        { }

        /// <inheritdoc/>
        public override void ValidateSchema(LogicalSchema schema)
        {
            if (Schema.Type.Long != schema.BaseSchema.Tag)
                throw new AIngleTypeException("'time-micros' can only be used with an underlying long type");
        }

        /// <inheritdoc/>
        public override object ConvertToBaseValue(object logicalValue, LogicalSchema schema)
        {
            var time = (TimeSpan)logicalValue;

            if (time > _maxTime)
                throw new ArgumentOutOfRangeException(nameof(logicalValue), "A 'time-micros' value can only have the range '00:00:00' to '23:59:59'.");

            return (long)(time - UnixEpochDateTime.TimeOfDay).TotalMilliseconds * 1000;
        }

        /// <inheritdoc/>
        public override object ConvertToLogicalValue(object baseValue, LogicalSchema schema)
        {
            var noMs = (long)baseValue / 1000;
            return UnixEpochDateTime.TimeOfDay.Add(TimeSpan.FromMilliseconds(noMs));
        }
    }
}
