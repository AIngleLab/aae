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
	
	public partial class Wide : ISpecificRecord
	{
		public static Schema _SCHEMA = AIngle.Schema.Parse(@"{""type"":""record"",""name"":""Wide"",""namespace"":""com.foo"",""fields"":[{""name"":""myInt"",""type"":""int""},{""name"":""myLong"",""type"":""long""},{""name"":""myBool"",""type"":""boolean""},{""name"":""myDouble"",""type"":""double""},{""name"":""myFloat"",""type"":""float""},{""name"":""myBytes"",""type"":""bytes""},{""name"":""myString"",""type"":""string""},{""name"":""myA"",""type"":{""type"":""record"",""name"":""A"",""namespace"":""com.foo"",""fields"":[{""name"":""f1"",""type"":""long""}]}},{""name"":""myE"",""type"":{""type"":""enum"",""name"":""MyEnum"",""namespace"":""com.foo"",""symbols"":[""A"",""B"",""C""]}},{""name"":""myInt2"",""type"":""int""},{""name"":""myLong2"",""type"":""long""},{""name"":""myBool2"",""type"":""boolean""},{""name"":""myDouble2"",""type"":""double""},{""name"":""myFloat2"",""type"":""float""},{""name"":""myBytes2"",""type"":""bytes""},{""name"":""myString2"",""type"":""string""},{""name"":""myA2"",""type"":""A""},{""name"":""myE2"",""type"":""MyEnum""},{""name"":""myInt3"",""type"":""int""},{""name"":""myLong3"",""type"":""long""},{""name"":""myBool3"",""type"":""boolean""},{""name"":""myDouble3"",""type"":""double""},{""name"":""myFloat3"",""type"":""float""},{""name"":""myBytes3"",""type"":""bytes""},{""name"":""myString3"",""type"":""string""},{""name"":""myA3"",""type"":""A""},{""name"":""myE3"",""type"":""MyEnum""},{""name"":""myInt4"",""type"":""int""},{""name"":""myLong4"",""type"":""long""},{""name"":""myBool4"",""type"":""boolean""},{""name"":""myDouble4"",""type"":""double""},{""name"":""myFloat4"",""type"":""float""},{""name"":""myBytes4"",""type"":""bytes""},{""name"":""myString4"",""type"":""string""},{""name"":""myA4"",""type"":""A""},{""name"":""myE4"",""type"":""MyEnum""}]}");
		private int _myInt;
		private long _myLong;
		private bool _myBool;
		private double _myDouble;
		private float _myFloat;
		private byte[] _myBytes;
		private string _myString;
		private com.foo.A _myA;
		private com.foo.MyEnum _myE;
		private int _myInt2;
		private long _myLong2;
		private bool _myBool2;
		private double _myDouble2;
		private float _myFloat2;
		private byte[] _myBytes2;
		private string _myString2;
		private com.foo.A _myA2;
		private com.foo.MyEnum _myE2;
		private int _myInt3;
		private long _myLong3;
		private bool _myBool3;
		private double _myDouble3;
		private float _myFloat3;
		private byte[] _myBytes3;
		private string _myString3;
		private com.foo.A _myA3;
		private com.foo.MyEnum _myE3;
		private int _myInt4;
		private long _myLong4;
		private bool _myBool4;
		private double _myDouble4;
		private float _myFloat4;
		private byte[] _myBytes4;
		private string _myString4;
		private com.foo.A _myA4;
		private com.foo.MyEnum _myE4;
		public virtual Schema Schema
		{
			get
			{
				return Wide._SCHEMA;
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
		public bool myBool
		{
			get
			{
				return this._myBool;
			}
			set
			{
				this._myBool = value;
			}
		}
		public double myDouble
		{
			get
			{
				return this._myDouble;
			}
			set
			{
				this._myDouble = value;
			}
		}
		public float myFloat
		{
			get
			{
				return this._myFloat;
			}
			set
			{
				this._myFloat = value;
			}
		}
		public byte[] myBytes
		{
			get
			{
				return this._myBytes;
			}
			set
			{
				this._myBytes = value;
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
		public com.foo.A myA
		{
			get
			{
				return this._myA;
			}
			set
			{
				this._myA = value;
			}
		}
		public com.foo.MyEnum myE
		{
			get
			{
				return this._myE;
			}
			set
			{
				this._myE = value;
			}
		}
		public int myInt2
		{
			get
			{
				return this._myInt2;
			}
			set
			{
				this._myInt2 = value;
			}
		}
		public long myLong2
		{
			get
			{
				return this._myLong2;
			}
			set
			{
				this._myLong2 = value;
			}
		}
		public bool myBool2
		{
			get
			{
				return this._myBool2;
			}
			set
			{
				this._myBool2 = value;
			}
		}
		public double myDouble2
		{
			get
			{
				return this._myDouble2;
			}
			set
			{
				this._myDouble2 = value;
			}
		}
		public float myFloat2
		{
			get
			{
				return this._myFloat2;
			}
			set
			{
				this._myFloat2 = value;
			}
		}
		public byte[] myBytes2
		{
			get
			{
				return this._myBytes2;
			}
			set
			{
				this._myBytes2 = value;
			}
		}
		public string myString2
		{
			get
			{
				return this._myString2;
			}
			set
			{
				this._myString2 = value;
			}
		}
		public com.foo.A myA2
		{
			get
			{
				return this._myA2;
			}
			set
			{
				this._myA2 = value;
			}
		}
		public com.foo.MyEnum myE2
		{
			get
			{
				return this._myE2;
			}
			set
			{
				this._myE2 = value;
			}
		}
		public int myInt3
		{
			get
			{
				return this._myInt3;
			}
			set
			{
				this._myInt3 = value;
			}
		}
		public long myLong3
		{
			get
			{
				return this._myLong3;
			}
			set
			{
				this._myLong3 = value;
			}
		}
		public bool myBool3
		{
			get
			{
				return this._myBool3;
			}
			set
			{
				this._myBool3 = value;
			}
		}
		public double myDouble3
		{
			get
			{
				return this._myDouble3;
			}
			set
			{
				this._myDouble3 = value;
			}
		}
		public float myFloat3
		{
			get
			{
				return this._myFloat3;
			}
			set
			{
				this._myFloat3 = value;
			}
		}
		public byte[] myBytes3
		{
			get
			{
				return this._myBytes3;
			}
			set
			{
				this._myBytes3 = value;
			}
		}
		public string myString3
		{
			get
			{
				return this._myString3;
			}
			set
			{
				this._myString3 = value;
			}
		}
		public com.foo.A myA3
		{
			get
			{
				return this._myA3;
			}
			set
			{
				this._myA3 = value;
			}
		}
		public com.foo.MyEnum myE3
		{
			get
			{
				return this._myE3;
			}
			set
			{
				this._myE3 = value;
			}
		}
		public int myInt4
		{
			get
			{
				return this._myInt4;
			}
			set
			{
				this._myInt4 = value;
			}
		}
		public long myLong4
		{
			get
			{
				return this._myLong4;
			}
			set
			{
				this._myLong4 = value;
			}
		}
		public bool myBool4
		{
			get
			{
				return this._myBool4;
			}
			set
			{
				this._myBool4 = value;
			}
		}
		public double myDouble4
		{
			get
			{
				return this._myDouble4;
			}
			set
			{
				this._myDouble4 = value;
			}
		}
		public float myFloat4
		{
			get
			{
				return this._myFloat4;
			}
			set
			{
				this._myFloat4 = value;
			}
		}
		public byte[] myBytes4
		{
			get
			{
				return this._myBytes4;
			}
			set
			{
				this._myBytes4 = value;
			}
		}
		public string myString4
		{
			get
			{
				return this._myString4;
			}
			set
			{
				this._myString4 = value;
			}
		}
		public com.foo.A myA4
		{
			get
			{
				return this._myA4;
			}
			set
			{
				this._myA4 = value;
			}
		}
		public com.foo.MyEnum myE4
		{
			get
			{
				return this._myE4;
			}
			set
			{
				this._myE4 = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.myInt;
			case 1: return this.myLong;
			case 2: return this.myBool;
			case 3: return this.myDouble;
			case 4: return this.myFloat;
			case 5: return this.myBytes;
			case 6: return this.myString;
			case 7: return this.myA;
			case 8: return this.myE;
			case 9: return this.myInt2;
			case 10: return this.myLong2;
			case 11: return this.myBool2;
			case 12: return this.myDouble2;
			case 13: return this.myFloat2;
			case 14: return this.myBytes2;
			case 15: return this.myString2;
			case 16: return this.myA2;
			case 17: return this.myE2;
			case 18: return this.myInt3;
			case 19: return this.myLong3;
			case 20: return this.myBool3;
			case 21: return this.myDouble3;
			case 22: return this.myFloat3;
			case 23: return this.myBytes3;
			case 24: return this.myString3;
			case 25: return this.myA3;
			case 26: return this.myE3;
			case 27: return this.myInt4;
			case 28: return this.myLong4;
			case 29: return this.myBool4;
			case 30: return this.myDouble4;
			case 31: return this.myFloat4;
			case 32: return this.myBytes4;
			case 33: return this.myString4;
			case 34: return this.myA4;
			case 35: return this.myE4;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.myInt = (System.Int32)fieldValue; break;
			case 1: this.myLong = (System.Int64)fieldValue; break;
			case 2: this.myBool = (System.Boolean)fieldValue; break;
			case 3: this.myDouble = (System.Double)fieldValue; break;
			case 4: this.myFloat = (System.Single)fieldValue; break;
			case 5: this.myBytes = (System.Byte[])fieldValue; break;
			case 6: this.myString = (System.String)fieldValue; break;
			case 7: this.myA = (com.foo.A)fieldValue; break;
			case 8: this.myE = (com.foo.MyEnum)fieldValue; break;
			case 9: this.myInt2 = (System.Int32)fieldValue; break;
			case 10: this.myLong2 = (System.Int64)fieldValue; break;
			case 11: this.myBool2 = (System.Boolean)fieldValue; break;
			case 12: this.myDouble2 = (System.Double)fieldValue; break;
			case 13: this.myFloat2 = (System.Single)fieldValue; break;
			case 14: this.myBytes2 = (System.Byte[])fieldValue; break;
			case 15: this.myString2 = (System.String)fieldValue; break;
			case 16: this.myA2 = (com.foo.A)fieldValue; break;
			case 17: this.myE2 = (com.foo.MyEnum)fieldValue; break;
			case 18: this.myInt3 = (System.Int32)fieldValue; break;
			case 19: this.myLong3 = (System.Int64)fieldValue; break;
			case 20: this.myBool3 = (System.Boolean)fieldValue; break;
			case 21: this.myDouble3 = (System.Double)fieldValue; break;
			case 22: this.myFloat3 = (System.Single)fieldValue; break;
			case 23: this.myBytes3 = (System.Byte[])fieldValue; break;
			case 24: this.myString3 = (System.String)fieldValue; break;
			case 25: this.myA3 = (com.foo.A)fieldValue; break;
			case 26: this.myE3 = (com.foo.MyEnum)fieldValue; break;
			case 27: this.myInt4 = (System.Int32)fieldValue; break;
			case 28: this.myLong4 = (System.Int64)fieldValue; break;
			case 29: this.myBool4 = (System.Boolean)fieldValue; break;
			case 30: this.myDouble4 = (System.Double)fieldValue; break;
			case 31: this.myFloat4 = (System.Single)fieldValue; break;
			case 32: this.myBytes4 = (System.Byte[])fieldValue; break;
			case 33: this.myString4 = (System.String)fieldValue; break;
			case 34: this.myA4 = (com.foo.A)fieldValue; break;
			case 35: this.myE4 = (com.foo.MyEnum)fieldValue; break;
			default: throw new AIngleRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
