// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by ainglegen.exe, version 0.9.0.0
//    Changes to this file may cause incorrect behavior and will be lost if code
//    is regenerated
// </auto-generated>
// ------------------------------------------------------------------------------
namespace com.foo
{
	using System;
	using System.Collections.Generic;
	using System.Text;
	using AIngle;
	using AIngle.Specific;
	
	public partial class Narrow : ISpecificRecord
	{
		public static Schema _SCHEMA = AIngle.Schema.Parse("{\"type\":\"record\",\"name\":\"Narrow\",\"namespace\":\"com.foo\",\"fields\":[{\"name\":\"myInt\"," +
				"\"type\":\"int\"},{\"name\":\"myLong\",\"type\":\"long\"},{\"name\":\"myString\",\"type\":\"string\"" +
				"}]}");
		private int _myInt;
		private long _myLong;
		private string _myString;
		public virtual Schema Schema
		{
			get
			{
				return Narrow._SCHEMA;
			}
		}
		public int myInt
		{
			get
			{
				return this._myInt;
			}
			set
			{
				this._myInt = value;
			}
		}
		public long myLong
		{
			get
			{
				return this._myLong;
			}
			set
			{
				this._myLong = value;
			}
		}
		public string myString
		{
			get
			{
				return this._myString;
			}
			set
			{
				this._myString = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.myInt;
			case 1: return this.myLong;
			case 2: return this.myString;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.myInt = (System.Int32)fieldValue; break;
			case 1: this.myLong = (System.Int64)fieldValue; break;
			case 2: this.myString = (System.String)fieldValue; break;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
