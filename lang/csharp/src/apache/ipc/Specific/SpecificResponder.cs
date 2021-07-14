/**

 */

using System;
using System.Collections.Generic;
using System.Reflection;
using AIngle.Generic;
using AIngle.IO;
using AIngle.Specific;
using AIngle.ipc.Generic;

namespace AIngle.ipc.Specific
{
    public class SpecificResponder<T> : GenericResponder
        where T : class, ISpecificProtocol
    {
        private readonly T impl;

        public SpecificResponder(T impl)
            : base(impl.Protocol)
        {
            this.impl = impl;
        }

        public override object Respond(Message message, object request)
        {
            int numParams = message.Request.Fields.Count;
            var parameters = new Object[numParams];
            var parameterTypes = new Type[numParams];

            int i = 0;

            foreach (Field field in message.Request.Fields)
            {
                Type type = ObjectCreator.Instance.GetType(field.Schema);
                parameterTypes[i] = type;
                parameters[i] = ((GenericRecord) request)[field.Name];

                i++;
            }

            MethodInfo method = typeof (T).GetMethod(message.Name, parameterTypes);
            try
            {
                return method.Invoke(impl, parameters);
            }
            catch (TargetInvocationException ex)
            {
                throw ex.InnerException;
            }
        }

        public override void WriteError(Schema schema, object error, Encoder output)
        {
            new SpecificWriter<object>(schema).Write(error, output);
        }

        public override object ReadRequest(Schema actual, Schema expected, Decoder input)
        {
            return new SpecificReader<object>(actual, expected).Read(null, input);
        }

        public override void WriteResponse(Schema schema, object response, Encoder output)
        {
            new SpecificWriter<object>(schema).Write(response, output);
        }
    }
}