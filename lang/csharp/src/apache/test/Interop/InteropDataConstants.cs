/**

 */
using System.Collections.Generic;
using AIngle.File;

namespace AIngle.Test.Interop
{
    public class InteropDataConstants
    {
        public static readonly HashSet<string> SupportedCodecNames = new HashSet<string>
        {
            DataFileConstants.NullCodec,
            DataFileConstants.DeflateCodec
        };
    }
}