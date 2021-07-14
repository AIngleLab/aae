/*  

 */

namespace AIngle.Reflect
{
    /// <summary>
    /// Extension methods for ArraySchema - make helper metadata look more like a property
    /// </summary>
    public static class ArraySchemaExtensions
    {
        /// <summary>
        /// Return the name of the array helper
        /// </summary>
        /// <param name="ars">this</param>
        /// <returns>value of the helper metadata - null if it isnt present</returns>
        public static string GetHelper(this ArraySchema ars)
        {
            string s = null;
            s = ars.GetProperty("helper");
            if (s != null && s.Length > 2)
            {
                s = s.Substring(1, s.Length - 2);
            }
            else
            {
                s = null;
            }

            return s;
        }
    }
}
