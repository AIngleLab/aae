// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by ainglegen.exe, version 0.9.0.0
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
	
	public abstract class MailCallback : Mail
	{
		public abstract void send(org.apache.aingle.test.Message message, AIngle.IO.ICallback<System.String> callback);
	}
}
