/*  

 */
using System;
using System.Collections.Concurrent;
using AIngle;

namespace AIngle.Reflect
{
    /// <summary>
    /// Cache of enum types. Cache key is the schema fullname.
    /// </summary>
    public static class EnumCache
    {
        private static ConcurrentDictionary<string, Type> _nameEnumMap = new ConcurrentDictionary<string, Type>();

        /// <summary>
        /// Add and entry to the cache
        /// </summary>
        /// <param name="schema"></param>
        /// <param name="dotnetEnum"></param>
        public static void AddEnumNameMapItem(NamedSchema schema, Type dotnetEnum)
        {
            _nameEnumMap.TryAdd(schema.Fullname, dotnetEnum);
        }

        /// <summary>
        /// Lookup an entry in the cache - based on the schema fullname
        /// </summary>
        /// <param name="schema"></param>
        /// <returns></returns>
        public static Type GetEnumeration(NamedSchema schema)
        {
            Type t;
            if (!_nameEnumMap.TryGetValue(schema.Fullname, out t))
            {
                throw new AIngleException($"Couldnt find enumeration for aingle fullname: {schema.Fullname}");
            }

            return t;
        }
    }
}
