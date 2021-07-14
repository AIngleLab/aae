/*  

 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Concurrent;

namespace AIngle.Reflect
{
    /// <summary>
    /// Class to help serialize and deserialize arrays. Arrays need the following methods Count(), Add(), Clear().true
    /// This class allows these methods to be specified externally to the collection.
    /// </summary>
    public class ArrayHelper
    {
        private static Type _defaultType = typeof(List<>);

        /// <summary>
        /// Collection type to apply by default to all array objects. If not set this defaults to a generic List.
        /// </summary>
        public static Type DefaultType
        {
            get => _defaultType;
            set => _defaultType = value;
        }

        /// <summary>
        /// The array
        /// </summary>
        public IEnumerable Enumerable { get; set; }

        /// <summary>
        /// Return the number of elements in the array.
        /// </summary>
        /// <value></value>
        public virtual int Count()
        {
            IList e = (IList)Enumerable;
            return e.Count;
        }

        /// <summary>
        /// Add an element to the array.
        /// </summary>
        /// <param name="o">Element to add to the array.</param>
        public virtual void Add(object o)
        {
            IList e = (IList)Enumerable;
            e.Add(o);
        }

        /// <summary>
        /// Clear the array.
        /// </summary>
        /// <value></value>
        public virtual void Clear()
        {
            IList e = (IList)Enumerable;
            e.Clear();
        }

        /// <summary>
        /// Type of the array to create when deserializing
        /// </summary>
        public virtual Type ArrayType
        {
            get => _defaultType;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="enumerable">Enumerable to initialize this helper with.</param>
        public ArrayHelper(IEnumerable enumerable)
        {
            Enumerable = enumerable;
        }
    }
}
