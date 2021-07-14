/*

 */

using System;

namespace AIngle.Reflect
{
    /// <summary>
    /// Field converter using a Func
    /// </summary>
    /// <typeparam name="TAIngle">AIngle type</typeparam>
    /// <typeparam name="TProperty">Property type</typeparam>
    public class FuncFieldConverter<TAIngle, TProperty> : TypedFieldConverter<TAIngle, TProperty>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FuncFieldConverter{A, P}"/> class.
        /// </summary>
        /// <param name="from">Delegate to convert from C# type to AIngle type</param>
        /// <param name="to">Delegate to convert from AIngle type to C# type</param>
        public FuncFieldConverter(Func<TAIngle, Schema, TProperty> from, Func<TProperty, Schema, TAIngle> to)
        {
            _from = from;
            _to = to;
        }

        private Func<TAIngle, Schema, TProperty> _from;

        private Func<TProperty, Schema, TAIngle> _to;

        /// <summary>
        /// Inherited conversion method - call the Func.
        /// </summary>
        /// <param name="o"></param>
        /// <param name="s"></param>
        /// <returns></returns>
        public override TProperty From(TAIngle o, Schema s)
        {
            return _from(o, s);
        }

        /// <summary>
        /// Inherited conversion method - call the Func.
        /// </summary>
        /// <param name="o"></param>
        /// <param name="s"></param>
        /// <returns></returns>
        public override TAIngle To(TProperty o, Schema s)
        {
            return _to(o, s);
        }
    }
}
