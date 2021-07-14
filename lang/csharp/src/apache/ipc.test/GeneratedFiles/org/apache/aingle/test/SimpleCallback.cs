// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by ainglegen.vshost.exe, version 0.9.0.0
//    Changes to this file may cause incorrect behavior and will be lost if code
//    is regenerated
// </auto-generated>
// ------------------------------------------------------------------------------
namespace org.apache.aingle.test
{
	using System;
	using System.Collections.Generic;
	using System.Text;
	using AIngle;
	using AIngle.Specific;
	
	/// <summary>
	/// Protocol used for testing.
	/// </summary>
	public abstract class SimpleCallback : Simple
	{
		// Send a greeting
		public abstract void hello(string greeting, AIngle.IO.ICallback<System.String> callback);
		// Pretend you're in a cave!
		public abstract void echo(org.apache.aingle.test.TestRecord record, AIngle.IO.ICallback<org.apache.aingle.test.TestRecord> callback);
		public abstract void add(int arg1, int arg2, AIngle.IO.ICallback<System.Int32> callback);
		public abstract void echoBytes(byte[] data, AIngle.IO.ICallback<System.Byte[]> callback);
		// Always throws an error.
		public abstract void error(AIngle.IO.ICallback<System.Object> callback);
	}
}
