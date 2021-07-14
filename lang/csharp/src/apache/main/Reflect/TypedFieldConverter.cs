/*

 */

using System;

namespace AIngle.Reflect
{
    /// <summary>
    /// Constructor
    /// </summary>
    /// <typeparam name="TAIngle">AIngle type</typeparam>
    /// <typeparam name="TProperty">Property type</typeparam>
    public abstract class TypedFieldConverter<TAIngle, TProperty> : IAIngleFieldConverter
    {
        /// <summary>
        /// Convert from AIngle type to property type
        /// </summary>
        /// <param name="o">AIngle value</param>
        /// <param name="s">Schema</param>
        /// <returns>Property value</returns>
        public abstract TProperty From(TAIngle o, Schema s);

        /// <summary>
        /// Convert from property type to AIngle type
        /// </summary>
        /// <param name="o"></param>
        /// <param name="s"></param>
        /// <returns></returns>
        public abstract TAIngle To(TProperty o, Schema s);

        /// <summary>
        /// Implement untyped interface
        /// </summary>
        /// <param name="o"></param>
        /// <param name="s"></param>
        /// <returns></returns>
        public object FromAIngleType(object o, Schema s)
        {
            if (!typeof(TAIngle).IsAssignableFrom(o.GetType()))
            {
                throw new AIngleException($"Converter from {typeof(TAIngle).Name} to {typeof(TProperty).Name} cannot convert object of type {o.GetType().Name} to {typeof(TAIngle).Name}, object {o.ToString()}");
            }

            return From((TAIngle)o, s);
        }

        /// <summary>
        /// Implement untyped interface
        /// </summary>
        /// <returns></returns>
        public Type GetAIngleType()
        {
            return typeof(TAIngle);
        }

        /// <summary>
        /// Implement untyped interface
        /// </summary>
        /// <returns></returns>
        public Type GetPropertyType()
        {
            return typeof(TProperty);
        }

        /// <summary>
        /// Implement untyped interface
        /// </summary>
        /// <param name="o"></param>
        /// <param name="s"></param>
        /// <returns></returns>
        public object ToAIngleType(object o, Schema s)
        {
            if (!typeof(TProperty).IsAssignableFrom(o.GetType()))
            {
                throw new AIngleException($"Converter from {typeof(TAIngle).Name} to {typeof(TProperty).Name} cannot convert object of type {o.GetType().Name} to {typeof(TProperty).Name}, object {o.ToString()}");
            }

            return To((TProperty)o, s);
        }
    }
}
