/*

 */
using System;
using System.Collections.Generic;
using Newtonsoft.Json.Linq;

namespace AIngle
{
    /// <summary>
    /// Class for array type schemas
    /// </summary>
    public class ArraySchema : UnnamedSchema
    {
        /// <summary>
        /// Schema for the array 'type' attribute
        /// </summary>
        public Schema ItemSchema { get; set;  }

        /// <summary>
        /// Static class to return a new instance of ArraySchema
        /// </summary>
        /// <param name="jtok">JSON object for the array schema</param>
        /// <param name="props">dictionary that provides access to custom properties</param>
        /// <param name="names">list of named schemas already parsed</param>
        /// <param name="encspace">enclosing namespace for the array schema</param>
        /// <returns></returns>
        internal static ArraySchema NewInstance(JToken jtok, PropertyMap props, SchemaNames names, string encspace)
        {
            JToken jitem = jtok["items"];
            if (null == jitem) throw new AIngleTypeException($"Array does not have 'items' at '{jtok.Path}'");
            var schema = Schema.ParseJson(jitem, names, encspace);
            return new ArraySchema(schema, props);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="items">schema for the array items type</param>
        /// <param name="props">dictionary that provides access to custom properties</param>
        private ArraySchema(Schema items, PropertyMap props) : base(Type.Array, props)
        {
            if (null == items) throw new ArgumentNullException(nameof(items));
            this.ItemSchema = items;
        }

        /// <summary>
        /// Writes the array schema in JSON format
        /// </summary>
        /// <param name="writer">JSON writer</param>
        /// <param name="names">list of named schemas already written</param>
        /// <param name="encspace">enclosing namespace</param>
        protected internal override void WriteJsonFields(Newtonsoft.Json.JsonTextWriter writer, SchemaNames names, string encspace)
        {
            writer.WritePropertyName("items");
            ItemSchema.WriteJson(writer, names, encspace);
        }

        /// <summary>
        /// Checks if this schema can read data written by the given schema. Used for decoding data.
        /// </summary>
        /// <param name="writerSchema">writer schema</param>
        /// <returns>true if this and writer schema are compatible based on the AINGLE specification, false otherwise</returns>
        public override bool CanRead(Schema writerSchema)
        {
            if (writerSchema.Tag != Tag) return false;

            ArraySchema that = writerSchema as ArraySchema;
            return ItemSchema.CanRead(that.ItemSchema);
        }

        /// <summary>
        /// Function to compare equality of two array schemas
        /// </summary>
        /// <param name="obj">other array schema</param>
        /// <returns>true two schemas are equal, false otherwise</returns>
        public override bool Equals(object obj)
        {
            if (this == obj) return true;

            if (obj != null && obj is ArraySchema)
            {
                ArraySchema that = obj as ArraySchema;
                if (ItemSchema.Equals(that.ItemSchema))
                    return areEqual(that.Props, this.Props);
            }
            return false;
        }

        /// <summary>
        /// Hashcode function
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            return 29 * ItemSchema.GetHashCode() + getHashCode(Props);
        }
    }
}
