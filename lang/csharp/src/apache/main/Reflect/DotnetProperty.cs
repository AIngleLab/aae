/*  

 */

using System;
using System.Reflection;
using System.Collections;

namespace AIngle.Reflect
{
    internal class DotnetProperty
    {
        private PropertyInfo _property;

        public IAIngleFieldConverter Converter { get; set; }

        private bool IsPropertyCompatible(AIngle.Schema.Type schemaTag)
        {
            Type propType;

            if (Converter == null)
            {
                propType = _property.PropertyType;
            }
            else
            {
                propType = Converter.GetAIngleType();
            }

            switch (schemaTag)
            {
                case AIngle.Schema.Type.Null:
                    return (Nullable.GetUnderlyingType(propType) != null) || (!propType.IsValueType);
                case AIngle.Schema.Type.Boolean:
                    return propType == typeof(bool);
                case AIngle.Schema.Type.Int:
                    return propType == typeof(int);
                case AIngle.Schema.Type.Long:
                    return propType == typeof(long);
                case AIngle.Schema.Type.Float:
                    return propType == typeof(float);
                case AIngle.Schema.Type.Double:
                    return propType == typeof(double);
                case AIngle.Schema.Type.Bytes:
                    return propType == typeof(byte[]);
                case AIngle.Schema.Type.String:
                    return typeof(string).IsAssignableFrom(propType);
                case AIngle.Schema.Type.Record:
                    //TODO: this probably should work for struct too
                    return propType.IsClass;
                case AIngle.Schema.Type.Enumeration:
                    return propType.IsEnum;
                case AIngle.Schema.Type.Array:
                    return typeof(IEnumerable).IsAssignableFrom(propType);
                case AIngle.Schema.Type.Map:
                    return typeof(IDictionary).IsAssignableFrom(propType);
                case AIngle.Schema.Type.Union:
                    return true;
                case AIngle.Schema.Type.Fixed:
                    return propType == typeof(byte[]);
                case AIngle.Schema.Type.Error:
                    return propType.IsClass;
            }

            return false;
        }

        public DotnetProperty(PropertyInfo property, AIngle.Schema.Type schemaTag,  IAIngleFieldConverter converter, ClassCache cache)
        {
            _property = property;
            Converter = converter;

            if (!IsPropertyCompatible(schemaTag))
            {
                if (Converter == null)
                {
                    var c = cache.GetDefaultConverter(schemaTag, _property.PropertyType);
                    if (c != null)
                    {
                        Converter = c;
                        return;
                    }
                }

                throw new AIngleException($"Property {property.Name} in object {property.DeclaringType} isn't compatible with AIngle schema type {schemaTag}");
            }
        }

        public DotnetProperty(PropertyInfo property, AIngle.Schema.Type schemaTag, ClassCache cache)
            : this(property, schemaTag, null, cache)
        {
        }

        public virtual Type GetPropertyType()
        {
            if (Converter != null)
            {
                return Converter.GetAIngleType();
            }

            return _property.PropertyType;
        }

        public virtual object GetValue(object o, Schema s)
        {
            if (Converter != null)
            {
                return Converter.ToAIngleType(_property.GetValue(o), s);
            }

            return _property.GetValue(o);
        }

        public virtual void SetValue(object o, object v, Schema s)
        {
            if (Converter != null)
            {
                _property.SetValue(o, Converter.FromAIngleType(v, s));
            }
            else
            {
                _property.SetValue(o, v);
            }
        }
    }
}
