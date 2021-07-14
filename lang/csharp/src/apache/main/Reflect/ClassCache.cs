/*

 */

using System;
using System.Collections;
using System.Collections.Concurrent;

namespace AIngle.Reflect
{
    /// <summary>
    /// Class holds a cache of C# classes and their properties. The key for the cache is the schema full name.
    /// </summary>
    public class ClassCache
    {
        private static ConcurrentBag<IAIngleFieldConverter> _defaultConverters = new ConcurrentBag<IAIngleFieldConverter>();

        private ConcurrentDictionary<string, DotnetClass> _nameClassMap = new ConcurrentDictionary<string, DotnetClass>();

        private ConcurrentDictionary<string, Type> _nameArrayMap = new ConcurrentDictionary<string, Type>();

        private void AddClassNameMapItem(RecordSchema schema, Type dotnetClass)
        {
            if (schema != null && GetClass(schema) != null)
            {
                return;
            }

            if (!dotnetClass.IsClass)
            {
                throw new AIngleException($"Type {dotnetClass.Name} is not a class");
            }

            _nameClassMap.TryAdd(schema.Fullname, new DotnetClass(dotnetClass, schema, this));
        }

        /// <summary>
        /// Add a default field converter
        /// </summary>
        /// <param name="converter"></param>
        public static void AddDefaultConverter(IAIngleFieldConverter converter)
        {
            _defaultConverters.Add(converter);
        }

        /// <summary>
        /// Add a converter defined using Func&lt;&gt;. The converter will be used whenever the source and target types
        /// match and a specific attribute is not defined.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <typeparam name="TAIngle"></typeparam>
        /// <typeparam name="TProperty"></typeparam>
        public static void AddDefaultConverter<TAIngle, TProperty>(Func<TAIngle, Schema, TProperty> from, Func<TProperty, Schema, TAIngle> to)
        {
            _defaultConverters.Add(new FuncFieldConverter<TAIngle, TProperty>(from, to));
        }

        /// <summary>
        /// Find a default converter
        /// </summary>
        /// <param name="tag"></param>
        /// <param name="propType"></param>
        /// <returns>The first matching converter - null if there isnt one</returns>
        public IAIngleFieldConverter GetDefaultConverter(AIngle.Schema.Type tag, Type propType)
        {
            Type aingleType;
            switch (tag)
            {
                case AIngle.Schema.Type.Null:
                    return null;
                case AIngle.Schema.Type.Boolean:
                    aingleType = typeof(bool);
                    break;
                case AIngle.Schema.Type.Int:
                    aingleType = typeof(int);
                    break;
                case AIngle.Schema.Type.Long:
                    aingleType = typeof(long);
                    break;
                case AIngle.Schema.Type.Float:
                    aingleType = typeof(float);
                    break;
                case AIngle.Schema.Type.Double:
                    aingleType = typeof(double);
                    break;
                case AIngle.Schema.Type.Bytes:
                    aingleType = typeof(byte[]);
                    break;
                case AIngle.Schema.Type.String:
                    aingleType = typeof(string);
                    break;
                case AIngle.Schema.Type.Record:
                    return null;
                case AIngle.Schema.Type.Enumeration:
                    return null;
                case AIngle.Schema.Type.Array:
                    return null;
                case AIngle.Schema.Type.Map:
                    return null;
                case AIngle.Schema.Type.Union:
                    return null;
                case AIngle.Schema.Type.Fixed:
                    aingleType = typeof(byte[]);
                    break;
                case AIngle.Schema.Type.Error:
                    return null;
                default:
                    return null;
            }

            foreach (var c in _defaultConverters)
            {
                if (c.GetAIngleType() == aingleType && c.GetPropertyType() == propType)
                {
                    return c;
                }
            }

            return null;
        }

        /// <summary>
        /// Add an array helper. Array helpers are used for collections that are not generic lists.
        /// </summary>
        /// <param name="name">Name of the helper. Corresponds to metadata "helper" field in the schema.</param>
        /// <param name="helperType">Type of helper. Inherited from ArrayHelper</param>
        public void AddArrayHelper(string name, Type helperType)
        {
            if (!typeof(ArrayHelper).IsAssignableFrom(helperType))
            {
                throw new AIngleException($"{helperType.Name} is not an ArrayHelper");
            }

            _nameArrayMap.TryAdd(name, helperType);
        }

        /// <summary>
        /// Find an array helper for an array schema node.
        /// </summary>
        /// <param name="schema">Schema</param>
        /// <param name="enumerable">The array object. If it is null then Add(), Count() and Clear methods will throw exceptions.</param>
        /// <returns></returns>
        public ArrayHelper GetArrayHelper(ArraySchema schema, IEnumerable enumerable)
        {
            Type h;
            // note ArraySchema is unamed and doesnt have a FulllName, use "helper" metadata
            // metadata is json string, strip quotes
            string s = null;
            s = schema.GetHelper();

            if (s != null && _nameArrayMap.TryGetValue(s, out h))
            {
                return (ArrayHelper)Activator.CreateInstance(h, enumerable);
            }

            return (ArrayHelper)Activator.CreateInstance(typeof(ArrayHelper), enumerable);
        }

        /// <summary>
        /// Find a class that matches the schema full name.
        /// </summary>
        /// <param name="schema"></param>
        /// <returns></returns>
        public DotnetClass GetClass(RecordSchema schema)
        {
            DotnetClass c;
            if (!_nameClassMap.TryGetValue(schema.Fullname, out c))
            {
               return null;
            }

            return c;
        }

        /// <summary>
        /// Add an entry to the class cache.
        /// </summary>
        /// <param name="objType">Type of the C# class</param>
        /// <param name="s">Schema</param>
        public void LoadClassCache(Type objType, Schema s)
        {
            switch (s)
            {
                case RecordSchema rs:
                    if (!objType.IsClass)
                    {
                        throw new AIngleException($"Cant map scalar type {objType.Name} to record {rs.Fullname}");
                    }

                    if (typeof(byte[]).IsAssignableFrom(objType)
                        || typeof(string).IsAssignableFrom(objType)
                        || typeof(IEnumerable).IsAssignableFrom(objType)
                        || typeof(IDictionary).IsAssignableFrom(objType))
                    {
                        throw new AIngleException($"Cant map type {objType.Name} to record {rs.Fullname}");
                    }

                    AddClassNameMapItem(rs, objType);
                    var c = GetClass(rs);
                    foreach (var f in rs.Fields)
                    {
                        var t = c.GetPropertyType(f);
                        LoadClassCache(t, f.Schema);
                    }

                    break;
                case ArraySchema ars:
                    if (!typeof(IEnumerable).IsAssignableFrom(objType))
                    {
                        throw new AIngleException($"Cant map type {objType.Name} to array {ars.Name}");
                    }

                    if (!objType.IsGenericType)
                    {
                        throw new AIngleException($"{objType.Name} needs to be a generic type");
                    }

                    LoadClassCache(objType.GenericTypeArguments[0], ars.ItemSchema);
                    break;
                case MapSchema ms:
                    if (!typeof(IDictionary).IsAssignableFrom(objType))
                    {
                        throw new AIngleException($"Cant map type {objType.Name} to map {ms.Name}");
                    }

                    if (!objType.IsGenericType)
                    {
                        throw new AIngleException($"Cant map non-generic type {objType.Name} to map {ms.Name}");
                    }

                    if (!typeof(string).IsAssignableFrom(objType.GenericTypeArguments[0]))
                    {
                        throw new AIngleException($"First type parameter of {objType.Name} must be assignable to string");
                    }

                    LoadClassCache(objType.GenericTypeArguments[1], ms.ValueSchema);
                    break;
                case NamedSchema ns:
                    EnumCache.AddEnumNameMapItem(ns, objType);
                    break;
                case UnionSchema us:
                    if (us.Schemas.Count == 2 && (us.Schemas[0].Tag == Schema.Type.Null || us.Schemas[1].Tag == Schema.Type.Null) && objType.IsClass)
                    {
                        // in this case objType will match the non null type in the union
                        foreach (var o in us.Schemas)
                        {
                            if (o.Tag != Schema.Type.Null)
                            {
                                LoadClassCache(objType, o);
                            }
                        }

                    }
                    else
                    {
                        // check the schema types are registered
                        foreach (var o in us.Schemas)
                        {
                            if (o.Tag == Schema.Type.Record && GetClass(o as RecordSchema) == null)
                            {
                                throw new AIngleException($"Class for union record type {o.Fullname} is not registered. Create a ClassCache object and call LoadClassCache");
                            }
                        }
                    }

                    break;
            }
        }
    }
}
