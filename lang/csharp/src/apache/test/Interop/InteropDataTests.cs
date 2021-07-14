/**

 */
using System.IO;
using NUnit.Framework;
using AIngle.File;
using AIngle.Generic;

namespace AIngle.Test.Interop
{
    [TestFixture]
    [Category("Interop")]
    public class InteropDataTests
    {
        [TestCase("../../../../../../../../build/interop/data")]
        public void TestInterop(string inputDir)
        {
            // Resolve inputDir relative to the TestDirectory
            inputDir = Path.Combine(TestContext.CurrentContext.TestDirectory, inputDir);

            Assert.True(Directory.Exists(inputDir),
                "Input directory does not exist. Run `build.sh interop-data-generate` first.");

            foreach (var aingleFile in Directory.EnumerateFiles(inputDir, "*.aingle"))
            {
                var codec = Path.GetFileNameWithoutExtension(aingleFile).Split('_');
                if (1 < codec.Length && !InteropDataConstants.SupportedCodecNames.Contains(codec[1]))
                {
                    continue;
                }

                using(var reader = DataFileReader<GenericRecord>.OpenReader(aingleFile))
                {
                    int i = 0;
                    foreach (var record in reader.NextEntries)
                    {
                        i++;
                        Assert.IsNotNull(record);
                    }
                    Assert.AreNotEqual(0, i);
                }
            }
        }
    }
}