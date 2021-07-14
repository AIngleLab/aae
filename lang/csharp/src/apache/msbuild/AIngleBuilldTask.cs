/**

 */

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace AIngle.msbuild
{
    public class AIngleBuildTask : Task
    {
        public override bool Execute()
        {
            var codegen = new CodeGen();
            if (SchemaFiles != null)
            {
                foreach (var schemaFile in SchemaFiles)
                {
                    var schema = Schema.Parse(System.IO.File.ReadAllText(schemaFile.ItemSpec));
                    codegen.AddSchema(schema);
                }
            }
            if (ProtocolFiles != null)
            {
                foreach (var protocolFile in ProtocolFiles)
                {
                    var protocol = Protocol.Parse(System.IO.File.ReadAllText(protocolFile.ItemSpec));
                    codegen.AddProtocol(protocol);
                }
            }

            var generateCode = codegen.GenerateCode();
            var namespaces = generateCode.Namespaces;
            for (var i = namespaces.Count - 1; i >= 0; i--)
            {
                var types = namespaces[i].Types;
                for (var j = types.Count - 1; j >= 0; j--)
                {
                    Log.LogMessage("Generating {0}.{1}", namespaces[i].Name, types[j].Name);
                }
            }

            codegen.WriteTypes(OutDir.ItemSpec);
            return true;
        }

        public ITaskItem[] SchemaFiles { get; set; }
        public ITaskItem[] ProtocolFiles { get; set; }

        [Required]
        public ITaskItem OutDir { get; set; }
    }
}
