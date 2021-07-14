/*

 */
using System;

namespace AIngle
{
    class CodeGenException : AIngleException
    {
        public CodeGenException()
        {
        }

        public CodeGenException(string s)
            : base(s)
        {
        }

        public CodeGenException(string s, Exception inner)
            : base(s, inner)
        {
        }
    }
}
