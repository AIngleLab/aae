/* 

 */

 using System;

namespace AIngle.Reflect
{
    /// <summary>
    /// Converters can be added to properties with an AIngleField attribute. Converters convert between the
    /// property type and the aingle type.
    /// </summary>
    public interface IAIngleFieldConverter
    {
        /// <summary>
        /// Convert from the C# type to the aingle type
        /// </summary>
        /// <param name="o">Value to convert</param>
        /// <param name="s">Schema</param>
        /// <returns>Converted value</returns>
        object ToAIngleType(object o, Schema s);

        /// <summary>
        /// Convert from the aingle type to the C# type
        /// </summary>
        /// <param name="o">Value to convert</param>
        /// <param name="s">Schema</param>
        /// <returns>Converted value</returns>
        object FromAIngleType(object o, Schema s);

        /// <summary>
        /// AIngle type
        /// </summary>
        /// <returns></returns>
        Type GetAIngleType();

        /// <summary>
        /// Property type
        /// </summary>
        /// <returns></returns>
        Type GetPropertyType();
    }
}
