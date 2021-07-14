/*

 */

using System;

namespace AIngle.Util
{
    /// <summary>
    /// Base for all logical type implementations.
    /// </summary>
    public abstract class LogicalType
    {
        /// <summary>
        /// The logical type name.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Initializes the base logical type.
        /// </summary>
        /// <param name="name">The logical type name.</param>
        protected LogicalType(string name)
        {
            Name = name;
        }

        /// <summary>
        /// Applies logical type validation for a given logical schema.
        /// </summary>
        /// <param name="schema">The schema to be validated.</param>
        public virtual void ValidateSchema(LogicalSchema schema)
        { }

        /// <summary>
        /// Converts a logical value to an instance of its base type.
        /// </summary>
        /// <param name="logicalValue">The logical value to convert.</param>
        /// <param name="schema">The schema that represents the target of the conversion.</param>
        /// <returns>An object representing the encoded value of the base type.</returns>
        public abstract object ConvertToBaseValue(object logicalValue, LogicalSchema schema);

        /// <summary>
        /// Converts a base value to an instance of the logical type.
        /// </summary>
        /// <param name="baseValue">The base value to convert.</param>
        /// <param name="schema">The schema that represents the target of the conversion.</param>
        /// <returns>An object representing the encoded value of the logical type.</returns>
        public abstract object ConvertToLogicalValue(object baseValue, LogicalSchema schema);

        /// <summary>
        /// Retrieve the .NET type that is represented by the logical type implementation.
        /// </summary>
        /// <param name="nullible">A flag indicating whether it should be nullible.</param>
        public abstract Type GetCSharpType(bool nullible);

        /// <summary>
        /// Determines if a given object is an instance of the logical type.
        /// </summary>
        /// <param name="logicalValue">The logical value to test.</param>
        public abstract bool IsInstanceOfLogicalType(object logicalValue);
    }
}
