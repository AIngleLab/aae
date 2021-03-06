/*

 */
using System;

namespace AIngle.Generic
{
    /// <summary>
    /// The default type used by GenericReader and GenericWriter for objects for FixedSchema
    /// </summary>
    public class GenericFixed
    {
        /// <summary>
        /// Value of this fixed.
        /// </summary>
        protected readonly byte[] value;
        private FixedSchema schema;

        /// <summary>
        /// Schema for this fixed.
        /// </summary>
        public FixedSchema Schema
        {
            get
            {
                return schema;
            }

            set
            {
                if (!(value is FixedSchema))
                    throw new AIngleException("Schema " + value.Name + " in set is not FixedSchema");

                if ((value as FixedSchema).Size != this.value.Length)
                    throw new AIngleException("Schema " + value.Name + " Size " + (value as FixedSchema).Size + "is not equal to bytes length " + this.value.Length);

                schema = value;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="GenericFixed"/> class.
        /// </summary>
        /// <param name="schema">Schema for this fixed.</param>
        public GenericFixed(FixedSchema schema)
        {
            value = new byte[schema.Size];
            this.Schema = schema;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="GenericFixed"/> class with a value.
        /// </summary>
        /// <param name="schema">Schema for this fixed.</param>
        /// <param name="value">Value of the fixed.</param>
        public GenericFixed(FixedSchema schema, byte[] value)
        {
            this.value = new byte[schema.Size];
            this.Schema = schema;
            Value = value;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="GenericFixed"/> class with a size.
        /// </summary>
        /// <param name="size">Size of the fixed in bytes.</param>
        protected GenericFixed(uint size)
        {
            this.value = new byte[size];
        }

        /// <summary>
        /// Value of this fixed.
        /// </summary>
        public byte[] Value
        {
            get { return this.value; }
            set
            {
                if (value.Length == this.value.Length)
                {
                    Array.Copy(value, this.value, value.Length);
                    return;
                }
                throw new AIngleException("Invalid length for fixed: " + value.Length + ", (" + Schema + ")");
            }
        }

        /// <inheritdoc/>
        public override bool Equals(object obj)
        {
            if (this == obj) return true;
            if (obj != null && obj is GenericFixed)
            {
                GenericFixed that = obj as GenericFixed;
                if (that.Schema.Equals(this.Schema))
                {
                    for (int i = 0; i < value.Length; i++) if (this.value[i] != that.value[i]) return false;
                    return true;
                }
            }
            return false;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            int result = Schema.GetHashCode();
            foreach (byte b in value)
            {
                result += 23 * b;
            }
            return result;
        }
    }
}
