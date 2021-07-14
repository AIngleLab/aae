/* 

 */

using System;
using System.Reflection;

namespace AIngle.Reflect
{
    /// <summary>
    /// Attribute that specifies the mapping between an AIngle field and C# class property.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property, AllowMultiple = false)]
    public class AIngleFieldAttribute : Attribute
    {
        /// <summary>
        /// Name of the field in the AIngle Schema
        /// </summary>
        public string FieldName { get; set; }

        /// <summary>
        /// Convert the property into a standard AIngle type - e.g. DateTimeOffset to long
        /// </summary>
        public IAIngleFieldConverter Converter { get; set; }

        /// <summary>
        /// Attribute to hold a field name and optionally a converter
        /// </summary>
        /// <param name="fieldName"></param>
        /// <param name="converter"></param>
        public AIngleFieldAttribute(string fieldName, Type converter = null)
        {
            FieldName = fieldName;
            if (converter != null)
            {
                Converter = (IAIngleFieldConverter)Activator.CreateInstance(converter);
            }
        }

        /// <summary>
        /// Used in property name mapping to specify a property type converter for the attribute.
        /// </summary>
        /// <param name="converter"></param>
        public AIngleFieldAttribute(Type converter)
        {
            FieldName = null;
            if (converter != null)
            {
                Converter = (IAIngleFieldConverter)Activator.CreateInstance(converter);
            }
        }
    }
}
