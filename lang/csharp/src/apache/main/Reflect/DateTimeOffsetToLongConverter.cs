/*  

 */

using System;

namespace AIngle.Reflect
{
    /// <summary>
    /// Convert C# DateTimeOffset properties to long unix time
    /// </summary>
    public class DateTimeOffsetToLongConverter : IAIngleFieldConverter
    {
        /// <summary>
        /// Convert from DateTimeOffset to Unix long
        /// </summary>
        /// <param name="o">DateTimeOffset</param>
        /// <param name="s">Schema</param>
        /// <returns></returns>
        public object ToAIngleType(object o, Schema s)
        {
            var dt = (DateTimeOffset)o;
            return dt.ToUnixTimeMilliseconds();
        }

        /// <summary>
        /// Convert from Unix long to DateTimeOffset
        /// </summary>
        /// <param name="o">long</param>
        /// <param name="s">Schema</param>
        /// <returns></returns>
        public object FromAIngleType(object o, Schema s)
        {
            var dt = DateTimeOffset.FromUnixTimeMilliseconds((long)o);
            return dt;
        }

        /// <summary>
        /// AIngle type
        /// </summary>
        /// <returns></returns>
        public Type GetAIngleType()
        {
            return typeof(long);
        }

        /// <summary>
        /// Property type
        /// </summary>
        /// <returns></returns>
        public Type GetPropertyType()
        {
            return typeof(DateTimeOffset);
        }
    }
}
