/*

 */

using System;

namespace AIngle.Util
{
    /// <summary>
    /// The 'time-millis' logical type.
    /// </summary>
    public class TimeMillisecond : LogicalUnixEpochType<TimeSpan>
    {
        private static readonly TimeSpan _maxTime = new TimeSpan(23, 59, 59);

        /// <summary>
        /// The logical type name for TimeMillisecond.
        /// </summary>
        public static readonly string LogicalTypeName = "time-millis";

        /// <summary>
        /// Initializes a new TimeMillisecond logical type.
        /// </summary>
        public TimeMillisecond() : base(LogicalTypeName)
        { }

        /// <inheritdoc/>
        public override void ValidateSchema(LogicalSchema schema)
        {
            if (Schema.Type.Int != schema.BaseSchema.Tag)
                throw new AIngleTypeException("'time-millis' can only be used with an underlying int type");
        }

        /// <inheritdoc/>
        public override object ConvertToBaseValue(object logicalValue, LogicalSchema schema)
        {
            var time = (TimeSpan)logicalValue;

            if (time > _maxTime)
                throw new ArgumentOutOfRangeException(nameof(logicalValue), "A 'time-millis' value can only have the range '00:00:00' to '23:59:59'.");

            return (int)(time - UnixEpochDateTime.TimeOfDay).TotalMilliseconds;
        }

        /// <inheritdoc/>
        public override object ConvertToLogicalValue(object baseValue, LogicalSchema schema)
        {
            var noMs = (int)baseValue;
            return UnixEpochDateTime.TimeOfDay.Add(TimeSpan.FromMilliseconds(noMs));
        }
    }
}
