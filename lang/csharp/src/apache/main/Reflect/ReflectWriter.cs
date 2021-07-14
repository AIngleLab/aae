/* 

 */

using AIngle.IO;
using AIngle.Generic;

namespace AIngle.Reflect
{
    /// <summary>
    /// Generic wrapper class for writing data from specific objects
    /// </summary>
    /// <typeparam name="T">type name of specific object</typeparam>
    public class ReflectWriter<T> : DatumWriter<T>
    {
        /// <summary>
        /// Default writer
        /// </summary>
        public ReflectDefaultWriter Writer { get => _writer; }

        private readonly ReflectDefaultWriter _writer;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="schema"></param>
        /// <param name="cache"></param>
        public ReflectWriter(Schema schema, ClassCache cache = null)
            : this(new ReflectDefaultWriter(typeof(T), schema, cache))
        {
        }

        /// <summary>
        /// The schema
        /// </summary>
        public Schema Schema { get => _writer.Schema; }

        /// <summary>
        /// Constructor with already created default writer.
        /// </summary>
        /// <param name="writer"></param>
        public ReflectWriter(ReflectDefaultWriter writer)
        {
            _writer = writer;
        }

        /// <summary>
        /// Serializes the given object using this writer's schema.
        /// </summary>
        /// <param name="value">The value to be serialized</param>
        /// <param name="encoder">The encoder to use for serializing</param>
        public void Write(T value, Encoder encoder)
        {
            _writer.Write(value, encoder);
        }
    }
}
