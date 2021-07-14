/**

 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace AIngle.Test.Utils
{
    public class CaseFinder
    {
        private static string labelRegex = "[a-zA-Z][_a-zA-Z0-9]*";
        private static string newCaseName = "INPUT";
        private static string newCaseMarker = "<<" + newCaseName;
        private static string startLinePattern = "^<<("+labelRegex+")(.*)$";

        /// <summary>
        /// Scan test-case file <code>streamReader</code> looking for test subcases
        /// marked with <code>label</code>.  Any such cases are appended
        /// (in order) to the "cases" parameter.
        /// </summary>
        /// <param name="streamReader"></param>
        /// <param name="label"></param>
        /// <param name="cases"></param>
        /// <returns></returns>
        public static List<object[]> Find(StreamReader streamReader, string label, List<object[]> cases)
        {
            if (!Regex.IsMatch(label, "^" + labelRegex + "$"))
            {
                throw new ArgumentException("Bad case subcase label: " + label);
            }

            string subcaseMarker = "<<" + label;

            var line = streamReader.ReadLine();
            while (true)
            {
                while (line != null && !line.StartsWith(newCaseMarker))
                {
                    line = streamReader.ReadLine();
                }
                if (line == null)
                {
                    break;
                }
                string input = ProcessHereDoc(streamReader, line);

                if (label == newCaseName)
                {
                    cases.Add(new object[] { input, null });
                    line = streamReader.ReadLine();
                    continue;
                }

                do
                {
                    line = streamReader.ReadLine();
                } while (line != null && (!line.StartsWith(newCaseMarker) && !line.StartsWith(subcaseMarker)));

                if (line == null || line.StartsWith(newCaseMarker))
                {
                    continue;
                }

                string expectedOutput = ProcessHereDoc(streamReader, line);
                cases.Add(new object[] { input, expectedOutput });
            }
            return cases;
        }

        private static string ProcessHereDoc(StreamReader streamReader, string docStart)
        {
            var match = Regex.Match(docStart, startLinePattern);
            if (!match.Success)
            {
                throw new ArgumentException(string.Format("Wasn't given the start of a heredoc (\"{0}\")", docStart));
            }

            string docName = match.Groups[1].Value;

            // Determine if this is a single-line heredoc, and process if it is
            string singleLineText = match.Groups[2].Value;
            if (singleLineText.Length != 0)
            {
                if (!singleLineText.StartsWith(" "))
                {
                    throw new IOException(string.Format("Single-line heredoc missing initial space (\"{0}\")", docStart));
                }
                return singleLineText.Substring(1);
            }

            // Process multi-line heredocs
            var sb = new StringBuilder();
            string line = streamReader.ReadLine();
            string prevLine = string.Empty;
            bool firstTime = true;
            while (line != null && line != docName)
            {
                if (!firstTime)
                {
                    sb.Append(prevLine).Append("\n");
                }
                else
                {
                    firstTime = false;
                }
                prevLine = line;
                line = streamReader.ReadLine();
            }
            if (line == null)
            {
                throw new IOException(string.Format("Here document ({0}) terminated by end-of-file.", docName));
            }
            return sb.Append(prevLine).ToString();
        }
    }
}
