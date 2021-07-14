/*

 */
using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;

namespace AIngle
{
    /// <summary>
    /// Class for schemas of primitive types
    /// </summary>
    public sealed class PrimitiveSchema : UnnamedSchema
    {
        /// <summary>
        /// Constructor for primitive schema
        /// </summary>
        /// <param name="type"></param>
        /// <param name="props">dictionary that provides access to custom properties</param>
        private PrimitiveSchema(Type type, PropertyMap props) : base(type, props)
        {
        }

        /// <summary>
        /// Static function to return new instance of primitive schema
        /// </summary>
        /// <param name="type">primitive type</param>
        /// <param name="props">dictionary that provides access to custom properties</param>
        /// <returns></returns>
        public static PrimitiveSchema NewInstance(string type, PropertyMap props = null)
        {
            const string q = "\"";
            if (type.StartsWith(q, StringComparison.Ordinal)
                && type.EndsWith(q, StringComparison.Ordinal))
            {
                type = type.Substring(1, type.Length - 2);
            }

            switch (type)
            {
                case "null":
                    return new PrimitiveSchema(Schema.Type.Null, props);
                case "boolean":
                    return new PrimitiveSchema(Schema.Type.Boolean, props);
                case "int":
                    return new PrimitiveSchema(Schema.Type.Int, props);
                case "long":
                    return new PrimitiveSchema(Schema.Type.Long, props);
                case "float":
                    return new PrimitiveSchema(Schema.Type.Float, props);
                case "double":
                    return new PrimitiveSchema(Schema.Type.Double, props);
                case "bytes":
                    return new PrimitiveSchema(Schema.Type.Bytes, props);
                case "string":
                    return new PrimitiveSchema(Schema.Type.String, props);
                default:
                    return null;
            }
        }

        /// <summary>
        /// Writes primitive schema in JSON format
        /// </summary>
        /// <param name="w"></param>
        /// <param name="names"></param>
        /// <param name="encspace"></param>
        protected internal override void WriteJson(JsonTextWriter w, SchemaNames names, string encspace)
        {
            w.WriteValue(Name);
        }

        /// <summary>
        /// Checks if this schema can read data written by the given schema. Used for decoding data.
        /// </summary>
        /// <param name="writerSchema">writer schema</param>
        /// <returns>true if this and writer schema are compatible based on the AINGLE specification, false otherwise</returns>
        public override bool CanRead(Schema writerSchema)
        {
            if (writerSchema is UnionSchema || Tag == writerSchema.Tag) return true;
            Type t = writerSchema.Tag;
            switch (Tag)
            {
                case Type.Double:
                    return t == Type.Int || t == Type.Long || t == Type.Float;
                case Type.Float:
                    return t == Type.Int || t == Type.Long;
                case Type.Long:
                    return t == Type.Int;
                default:
                    return false;
            }
        }

        /// <summary>
        /// Function to compare equality of two primitive schemas
        /// </summary>
        /// <param name="obj">other primitive schema</param>
        /// <returns>true two schemas are equal, false otherwise</returns>
        public override bool Equals(object obj)
        {
            if (this == obj) return true;

            if (obj != null && obj is PrimitiveSchema)
            {
                var that = obj as PrimitiveSchema;
                if (this.Tag == that.Tag)
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
            return 13 * Tag.GetHashCode() + getHashCode(Props);
        }
    }
}
