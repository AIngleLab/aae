/*  

 */

using System;
using System.Reflection;
using System.Collections.Concurrent;
using AIngle;

namespace AIngle.Reflect
{
    /// <summary>
    /// Collection of DotNetProperty objects to repre
    /// </summary>
    public class DotnetClass
    {
        private ConcurrentDictionary<string, DotnetProperty> _propertyMap = new ConcurrentDictionary<string, DotnetProperty>();

        private Type _type;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="t">type of the class</param>
        /// <param name="r">record schema</param>
        /// <param name="cache">class cache - can be reused</param>
        public DotnetClass(Type t, RecordSchema r, ClassCache cache)
        {
            _type = t;
            foreach (var f in r.Fields)
            {
                bool hasAttribute = false;
                PropertyInfo prop = GetPropertyInfo(f);

                foreach (var attr in prop.GetCustomAttributes(true))
                {
                    var aingleAttr = attr as AIngleFieldAttribute;
                    if (aingleAttr != null)
                    {
                        hasAttribute = true;
                        _propertyMap.TryAdd(f.Name, new DotnetProperty(prop, f.Schema.Tag, aingleAttr.Converter, cache));
                        break;
                    }
                }

                if (!hasAttribute)
                {
                    _propertyMap.TryAdd(f.Name, new DotnetProperty(prop, f.Schema.Tag, cache));
                }
            }
        }

        private PropertyInfo GetPropertyInfo(Field f)
        {
            var prop = _type.GetProperty(f.Name);
            if (prop != null)
            {
                return prop;
            }
            foreach (var p in _type.GetProperties())
            {
                foreach (var attr in p.GetCustomAttributes(true))
                {
                    var aingleAttr = attr as AIngleFieldAttribute;
                    if (aingleAttr != null && aingleAttr.FieldName != null && aingleAttr.FieldName == f.Name)
                    {
                        return p;
                    }
                }
            }

            throw new AIngleException($"Class {_type.Name} doesnt contain property {f.Name}");
        }

        /// <summary>
        /// Return the value of a property from an object referenced by a field
        /// </summary>
        /// <param name="o">the object</param>
        /// <param name="f">FieldSchema used to look up the property</param>
        /// <returns></returns>
        public object GetValue(object o, Field f)
        {
            DotnetProperty p;
            if (!_propertyMap.TryGetValue(f.Name, out p))
            {
                throw new AIngleException($"ByPosClass doesnt contain property {f.Name}");
            }

            return p.GetValue(o, f.Schema);
        }

        /// <summary>
        /// Set the value of a property in a C# object
        /// </summary>
        /// <param name="o">the object</param>
        /// <param name="f">field schema</param>
        /// <param name="v">value for the proprty referenced by the field schema</param>
        public void SetValue(object o, Field f, object v)
        {
            DotnetProperty p;
            if (!_propertyMap.TryGetValue(f.Name, out p))
            {
                throw new AIngleException($"ByPosClass doesnt contain property {f.Name}");
            }

            p.SetValue(o, v, f.Schema);
        }

        /// <summary>
        /// Return the type of the Class
        /// </summary>
        /// <returns>The </returns>
        public Type GetClassType()
        {
            return _type;
        }

        /// <summary>
        /// Return the type of a property referenced by a field
        /// </summary>
        /// <param name="f"></param>
        /// <returns></returns>
        public Type GetPropertyType(Field f)
        {
            DotnetProperty p;
            if (!_propertyMap.TryGetValue(f.Name, out p))
            {
                throw new AIngleException($"ByPosClass doesnt contain property {f.Name}");
            }

            return p.GetPropertyType();
        }
    }
}
