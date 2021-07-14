/**

 */

using System;
using System.Collections.Generic;
using System.Linq;
namespace AIngle.Test
{
    public static class ExtensionMethods
    {
        public static bool SequenceEqual(this byte[] source, byte[] target)
        {
            if (source.Length != target.Length)
            {
                return false;
            }
            for (int i = 0; i < source.Length; i++)
            {
                if (source[i] != target[i])
                {
                    return false;
                }
            }
            return true;
        }

        public static void ForEach<T1,T2>( this IEnumerable<T1> e1, IEnumerable<T2> e2, Action<T1,T2> action)
        {
            foreach(var items in e1.Zip(e2, Tuple.Create))
            {
                action(items.Item1, items.Item2);
            }
        }
    }
}
