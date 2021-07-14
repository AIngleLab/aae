/*

 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AIngle.Generic;

namespace AIngle.Specific
{
    /// <summary>
    /// Base class for all generated classes
    /// </summary>
    public abstract class SpecificFixed : GenericFixed
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SpecificFixed"/> class.
        /// </summary>
        /// <param name="size"></param>
        public SpecificFixed(uint size) : base(size) { }

        /// <summary>
        /// Schema of this instance.
        /// </summary>
        public abstract new Schema Schema { get; }

        /// <summary>
        /// Determines whether the provided fixed is equivalent this this instance.
        /// </summary>
        /// <param name="obj">Fixed to compare.</param>
        /// <returns>True if the fixed instances have equal values.</returns>
        protected bool Equals(SpecificFixed obj)
        {
            if (this == obj) return true;
            if (obj != null && obj is SpecificFixed)
            {
                SpecificFixed that = obj as SpecificFixed;
                if (that.Schema.Equals(this.Schema))
                {
                    for (int i = 0; i < value.Length; i++) if (this.value[i] != that.Value[i]) return false;
                    return true;
                }
            }
            return false;

        }

        /// <inheritdoc/>
        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != this.GetType()) return false;
            return Equals((SpecificFixed) obj);
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
