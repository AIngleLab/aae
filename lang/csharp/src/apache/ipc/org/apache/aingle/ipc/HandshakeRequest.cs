// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by MSBuild.exe, version 0.9.0.0
//    Changes to this file may cause incorrect behavior and will be lost if code
//    is regenerated
// </auto-generated>
// ------------------------------------------------------------------------------
namespace org.apache.aingle.ipc
{
	using System;
	using System.Collections.Generic;
	using System.Text;
	using AIngle;
	using AIngle.Specific;
	
	public partial class HandshakeRequest : ISpecificRecord
	{
		private static Schema _SCHEMA = AIngle.Schema.Parse(@"{""type"":""record"",""name"":""HandshakeRequest"",""namespace"":""org.apache.aingle.ipc"",""fields"":[{""name"":""clientHash"",""type"":{""type"":""fixed"",""name"":""MD5"",""namespace"":""org.apache.aingle.ipc"",""size"":16}},{""name"":""clientProtocol"",""type"":[""null"",""string""]},{""name"":""serverHash"",""type"":""MD5""},{""name"":""meta"",""type"":[""null"",{""type"":""map"",""values"":""bytes""}]}]}");
		private org.apache.aingle.ipc.MD5 _clientHash;
		private string _clientProtocol;
		private org.apache.aingle.ipc.MD5 _serverHash;
		private IDictionary<string,System.Byte[]> _meta;
		public virtual Schema Schema
		{
			get
			{
				return HandshakeRequest._SCHEMA;
			}
		}
		public org.apache.aingle.ipc.MD5 clientHash
		{
			get
			{
				return this._clientHash;
			}
			set
			{
				this._clientHash = value;
			}
		}
		public string clientProtocol
		{
			get
			{
				return this._clientProtocol;
			}
			set
			{
				this._clientProtocol = value;
			}
		}
		public org.apache.aingle.ipc.MD5 serverHash
		{
			get
			{
				return this._serverHash;
			}
			set
			{
				this._serverHash = value;
			}
		}
		public IDictionary<string,System.Byte[]> meta
		{
			get
			{
				return this._meta;
			}
			set
			{
				this._meta = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.clientHash;
			case 1: return this.clientProtocol;
			case 2: return this.serverHash;
			case 3: return this.meta;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.clientHash = (org.apache.aingle.ipc.MD5)fieldValue; break;
			case 1: this.clientProtocol = (System.String)fieldValue; break;
			case 2: this.serverHash = (org.apache.aingle.ipc.MD5)fieldValue; break;
			case 3: this.meta = (IDictionary<string,System.Byte[]>)fieldValue; break;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
