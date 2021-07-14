/*

 */
using System.Globalization;

namespace AIngle
{
    /// <summary>
    /// Base class for all unnamed schemas
    /// </summary>
    public abstract class UnnamedSchema : Schema
    {
        /// <summary>
        /// Base constructor for an <see cref="UnnamedSchema"/>.
        /// </summary>
        /// <param name="type">Type of schema.</param>
        /// <param name="props">Dictionary that provides access to custom properties</param>
        protected UnnamedSchema(Type type, PropertyMap props) : base(type, props)
        {
        }

        /// <inheritdoc/>
        public override string Name
        {
            get { return Tag.ToString().ToLower(CultureInfo.InvariantCulture); }
        }
    }
}
