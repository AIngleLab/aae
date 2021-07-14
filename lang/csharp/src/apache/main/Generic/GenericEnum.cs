/*

 */

namespace AIngle.Generic
{
    /// <summary>
    /// The default class to hold values for enum schema in GenericReader and GenericWriter.
    /// </summary>
    public class GenericEnum
    {
        /// <summary>
        /// Schema for this enum.
        /// </summary>
        public EnumSchema Schema { get; private set; }

        private string value;

        /// <summary>
        /// Value of the enum.
        /// </summary>
        public string Value {
            get { return value; }
            set
            {
                if (!Schema.Contains(value))
                {
                    if (!string.IsNullOrEmpty(Schema.Default))
                    {
                        this.value = Schema.Default;
                    }
                    else
                    {
                        throw new AIngleException("Unknown value for enum: " + value + "(" + Schema + ")");
                    }
                }
                else
                {
                    this.value = value;
                }
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="GenericEnum"/> class.
        /// </summary>
        /// <param name="schema">Schema for this enum.</param>
        /// <param name="value">Value of the enum.</param>
        public GenericEnum(EnumSchema schema, string value)
        {
            this.Schema = schema;
            this.Value = value;
        }

        /// <inheritdoc/>
        public override bool Equals(object obj)
        {
            if (obj == this) return true;
            return (obj != null && obj is GenericEnum)
                ? Value.Equals((obj as GenericEnum).Value, System.StringComparison.Ordinal)
                : false;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
#pragma warning disable CA1307 // Specify StringComparison
            return 17 * Value.GetHashCode();
#pragma warning restore CA1307 // Specify StringComparison
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "Schema: " + Schema + ", value: " + Value;
        }
    }
}
