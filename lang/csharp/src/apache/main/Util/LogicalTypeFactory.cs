/*

 */

using System.Collections.Generic;

namespace AIngle.Util
{
    /// <summary>
    /// A factory for logical type implementations.
    /// </summary>
    public class LogicalTypeFactory
    {
        private readonly IDictionary<string, LogicalType> _logicalTypes;

        /// <summary>
        /// Returns the <see cref="LogicalTypeFactory" /> singleton.
        /// </summary>
        /// <returns>The <see cref="LogicalTypeFactory" /> singleton. </returns>
        public static LogicalTypeFactory Instance { get; } = new LogicalTypeFactory();

        private LogicalTypeFactory()
        {
            _logicalTypes = new Dictionary<string, LogicalType>()
            {
                { Decimal.LogicalTypeName, new Decimal() },
                { Date.LogicalTypeName, new Date() },
                { TimeMillisecond.LogicalTypeName, new TimeMillisecond() },
                { TimeMicrosecond.LogicalTypeName, new TimeMicrosecond() },
                { TimestampMillisecond.LogicalTypeName, new TimestampMillisecond() },
                { TimestampMicrosecond.LogicalTypeName, new TimestampMicrosecond() },
                { Uuid.LogicalTypeName, new Uuid() }
            };
        }

        /// <summary>
        /// Registers or replaces a logical type implementation.
        /// </summary>
        /// <param name="logicalType">The <see cref="LogicalType"/> implementation that should be registered.</param>
        public void Register(LogicalType logicalType)
        {
            _logicalTypes[logicalType.Name] = logicalType;
        }

        /// <summary>
        /// Retrieves a logical type implementation for a given logical schema.
        /// </summary>
        /// <param name="schema">The schema.</param>
        /// <param name="ignoreInvalidOrUnknown">A flag to indicate if an exception should be thrown for invalid
        /// or unknown logical types.</param>
        /// <returns>A <see cref="LogicalType" />.</returns>
        public LogicalType GetFromLogicalSchema(LogicalSchema schema, bool ignoreInvalidOrUnknown = false)
        {
            try
            {
                if (!_logicalTypes.TryGetValue(schema.LogicalTypeName, out LogicalType logicalType))
                    throw new AIngleTypeException("Logical type '" + schema.LogicalTypeName + "' is not supported.");

                logicalType.ValidateSchema(schema);

                return logicalType;
            }
            catch (AIngleTypeException)
            {
                if (!ignoreInvalidOrUnknown)
                    throw;
            }

            return null;
        }
    }
}
