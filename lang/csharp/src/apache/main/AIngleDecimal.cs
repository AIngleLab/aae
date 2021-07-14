/*

 */

using System;
using System.Globalization;
using System.Numerics;

namespace AIngle
{
    /// <summary>
    /// Represents a big decimal.
    /// </summary>
    #pragma warning disable CS1591 // Missing XML comment for publicly visible type or member
    #pragma warning disable CA2225 // Operator overloads have named alternates
    public struct AIngleDecimal : IConvertible, IFormattable, IComparable, IComparable<AIngleDecimal>, IEquatable<AIngleDecimal>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given double.
        /// </summary>
        /// <param name="value">The double value.</param>
        public AIngleDecimal(double value)
            : this((decimal)value)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given float.
        /// </summary>
        /// <param name="value">The float value.</param>
        public AIngleDecimal(float value)
            : this((decimal)value)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given decimal.
        /// </summary>
        /// <param name="value">The decimal value.</param>
        public AIngleDecimal(decimal value)
        {
            var bytes = GetBytesFromDecimal(value);

            var unscaledValueBytes = new byte[12];
            Array.Copy(bytes, unscaledValueBytes, unscaledValueBytes.Length);

            var unscaledValue = new BigInteger(unscaledValueBytes);
            var scale = bytes[14];

            if (bytes[15] == 128)
                unscaledValue *= BigInteger.MinusOne;

            UnscaledValue = unscaledValue;
            Scale = scale;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given int.
        /// </summary>
        /// <param name="value">The int value.</param>
        public AIngleDecimal(int value)
            : this(new BigInteger(value), 0) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given long.
        /// </summary>
        /// <param name="value">The long value.</param>
        public AIngleDecimal(long value)
            : this(new BigInteger(value), 0) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given unsigned int.
        /// </summary>
        /// <param name="value">The unsigned int value.</param>
        public AIngleDecimal(uint value)
            : this(new BigInteger(value), 0) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given unsigned long.
        /// </summary>
        /// <param name="value">The unsigned long value.</param>
        public AIngleDecimal(ulong value)
            : this(new BigInteger(value), 0) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="AIngleDecimal"/> class from a given <see cref="BigInteger"/>
        /// and a scale.
        /// </summary>
        /// <param name="unscaledValue">The double value.</param>
        /// <param name="scale">The scale.</param>
        public AIngleDecimal(BigInteger unscaledValue, int scale)
        {
            UnscaledValue = unscaledValue;
            Scale = scale;
        }

        /// <summary>
        /// Gets the unscaled integer value represented by the current <see cref="AIngleDecimal"/>.
        /// </summary>
        public BigInteger UnscaledValue { get; }

        /// <summary>
        /// Gets the scale of the current <see cref="AIngleDecimal"/>.
        /// </summary>
        public int Scale { get; }

        /// <summary>
        /// Gets the sign of the current <see cref="AIngleDecimal"/>.
        /// </summary>
        internal int Sign
        {
            get { return UnscaledValue.Sign; }
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a string.
        /// </summary>
        /// <returns>A string representation of the numeric value.</returns>
        public override string ToString()
        {
            var number = UnscaledValue.ToString(CultureInfo.CurrentCulture);

            if (Scale > 0)
                return number.Insert(number.Length - Scale, CultureInfo.CurrentCulture.NumberFormat.NumberDecimalSeparator);

            return number;
        }

        public static bool operator ==(AIngleDecimal left, AIngleDecimal right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(AIngleDecimal left, AIngleDecimal right)
        {
            return !left.Equals(right);
        }

        public static bool operator >(AIngleDecimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) > 0;
        }

        public static bool operator >=(AIngleDecimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) >= 0;
        }

        public static bool operator <(AIngleDecimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) < 0;
        }

        public static bool operator <=(AIngleDecimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) <= 0;
        }

        public static bool operator ==(AIngleDecimal left, decimal right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(AIngleDecimal left, decimal right)
        {
            return !left.Equals(right);
        }

        public static bool operator >(AIngleDecimal left, decimal right)
        {
            return left.CompareTo(right) > 0;
        }

        public static bool operator >=(AIngleDecimal left, decimal right)
        {
            return left.CompareTo(right) >= 0;
        }

        public static bool operator <(AIngleDecimal left, decimal right)
        {
            return left.CompareTo(right) < 0;
        }

        public static bool operator <=(AIngleDecimal left, decimal right)
        {
            return left.CompareTo(right) <= 0;
        }

        public static bool operator ==(decimal left, AIngleDecimal right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(decimal left, AIngleDecimal right)
        {
            return !left.Equals(right);
        }

        public static bool operator >(decimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) > 0;
        }

        public static bool operator >=(decimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) >= 0;
        }

        public static bool operator <(decimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) < 0;
        }

        public static bool operator <=(decimal left, AIngleDecimal right)
        {
            return left.CompareTo(right) <= 0;
        }

        public static explicit operator byte(AIngleDecimal value)
        {
            return ToByte(value);
        }

        /// <summary>
        /// Creates a byte from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A byte.</returns>
        public static byte ToByte(AIngleDecimal value)
        {
            return value.ToType<byte>();
        }

        public static explicit operator sbyte(AIngleDecimal value)
        {
            return ToSByte(value);
        }

        /// <summary>
        /// Creates a signed byte from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A signed byte.</returns>
        public static sbyte ToSByte(AIngleDecimal value)
        {
            return value.ToType<sbyte>();
        }

        public static explicit operator short(AIngleDecimal value)
        {
            return ToInt16(value);
        }

        /// <summary>
        /// Creates a short from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A short.</returns>
        public static short ToInt16(AIngleDecimal value)
        {
            return value.ToType<short>();
        }

        public static explicit operator int(AIngleDecimal value)
        {
            return ToInt32(value);
        }

        /// <summary>
        /// Creates an int from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>An int.</returns>
        public static int ToInt32(AIngleDecimal value)
        {
            return value.ToType<int>();
        }

        public static explicit operator long(AIngleDecimal value)
        {
            return ToInt64(value);
        }

        /// <summary>
        /// Creates a long from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A long.</returns>
        public static long ToInt64(AIngleDecimal value)
        {
            return value.ToType<long>();
        }

        public static explicit operator ushort(AIngleDecimal value)
        {
            return ToUInt16(value);
        }

        /// <summary>
        /// Creates an unsigned short from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>An unsigned short.</returns>
        public static ushort ToUInt16(AIngleDecimal value)
        {
            return value.ToType<ushort>();
        }

        public static explicit operator uint(AIngleDecimal value)
        {
            return ToUInt32(value);
        }

        /// <summary>
        /// Creates an unsigned int from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>An unsigned int.</returns>
        public static uint ToUInt32(AIngleDecimal value)
        {
            return value.ToType<uint>();
        }

        public static explicit operator ulong(AIngleDecimal value)
        {
            return ToUInt64(value);
        }

        /// <summary>
        /// Creates an unsigned long from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>An unsigned long.</returns>
        public static ulong ToUInt64(AIngleDecimal value)
        {
            return value.ToType<ulong>();
        }

        public static explicit operator float(AIngleDecimal value)
        {
            return ToSingle(value);
        }

        /// <summary>
        /// Creates a double from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A double.</returns>
        public static float ToSingle(AIngleDecimal value)
        {
            return value.ToType<float>();
        }

        public static explicit operator double(AIngleDecimal value)
        {
            return ToDouble(value);
        }

        /// <summary>
        /// Creates a double from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A double.</returns>
        public static double ToDouble(AIngleDecimal value)
        {
            return value.ToType<double>();
        }

        public static explicit operator decimal(AIngleDecimal value)
        {
            return ToDecimal(value);
        }

        /// <summary>
        /// Creates a decimal from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A decimal.</returns>
        public static decimal ToDecimal(AIngleDecimal value)
        {
            return value.ToType<decimal>();
        }

        public static explicit operator BigInteger(AIngleDecimal value)
        {
            return ToBigInteger(value);
        }

        /// <summary>
        /// Creates a <see cref="BigInteger"/> from a given <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="value">The <see cref="AIngleDecimal"/>.</param>
        /// <returns>A <see cref="BigInteger"/>.</returns>
        public static BigInteger ToBigInteger(AIngleDecimal value)
        {
            var scaleDivisor = BigInteger.Pow(new BigInteger(10), value.Scale);
            var scaledValue = BigInteger.Divide(value.UnscaledValue, scaleDivisor);
            return scaledValue;
        }

        public static implicit operator AIngleDecimal(byte value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(sbyte value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(short value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(int value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(long value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(ushort value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(uint value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(ulong value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(float value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(double value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(decimal value)
        {
            return new AIngleDecimal(value);
        }

        public static implicit operator AIngleDecimal(BigInteger value)
        {
            return new AIngleDecimal(value, 0);
        } 

        /// <summary>
        /// Converts the numeric value of the current <see cref="AIngleDecimal"/> to a given type.
        /// </summary>
        /// <typeparam name="T">The type to which the value of the current <see cref="AIngleDecimal"/> should be converted.</typeparam>
        /// <returns>A value of type <typeparamref name="T"/> converted from the current <see cref="AIngleDecimal"/>.</returns>
        public T ToType<T>()
            where T : struct
        {
            return (T)((IConvertible)this).ToType(typeof(T), null);
        }

        /// <summary>
        /// Converts the numeric value of the current <see cref="AIngleDecimal"/> to a given type.
        /// </summary>
        /// <param name="conversionType">The type to which the value of the current <see cref="AIngleDecimal"/> should be converted.</param>
        /// <param name="provider">An System.IFormatProvider interface implementation that supplies culture-specific formatting information.</param>
        /// <returns></returns>
        object IConvertible.ToType(Type conversionType, IFormatProvider provider)
        {
            var scaleDivisor = BigInteger.Pow(new BigInteger(10), Scale);
            var remainder = BigInteger.Remainder(UnscaledValue, scaleDivisor);
            var scaledValue = BigInteger.Divide(UnscaledValue, scaleDivisor);

            if (scaledValue > new BigInteger(Decimal.MaxValue))
                throw new ArgumentOutOfRangeException("value", "The value " + UnscaledValue + " cannot fit into " + conversionType.Name + ".");

            var leftOfDecimal = (decimal)scaledValue;
            var rightOfDecimal = ((decimal)remainder) / ((decimal)scaleDivisor);

            var value = leftOfDecimal + rightOfDecimal;
            return Convert.ChangeType(value, conversionType, provider);
        }

        /// <summary>
        /// Returns a value that indicates whether the current <see cref="AIngleDecimal"/> and a specified object
        /// have the same value.
        /// </summary>
        /// <param name="obj">The object to compare.</param>
        /// <returns>true if the obj argument is an <see cref="AIngleDecimal"/> object, and its value
        /// is equal to the value of the current <see cref="AIngleDecimal"/> instance; otherwise false.
        /// </returns>
        public override bool Equals(object obj)
        {
            return (obj is AIngleDecimal) && Equals((AIngleDecimal)obj);
        }

        /// <summary>
        /// Returns the hash code for the current <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode()
        {
            return UnscaledValue.GetHashCode() ^ Scale.GetHashCode();
        }

        /// <summary>
        /// Returns the <see cref="TypeCode"/> for the current <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <returns><see cref="TypeCode.Object"/>.</returns>
        TypeCode IConvertible.GetTypeCode()
        {
            return TypeCode.Object;
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a boolean.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>true or false, which reflects the value of the current <see cref="AIngleDecimal"/>.</returns>
        bool IConvertible.ToBoolean(IFormatProvider provider)
        {
            return Convert.ToBoolean(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a byte.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A byte.</returns>
        byte IConvertible.ToByte(IFormatProvider provider)
        {
            return Convert.ToByte(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a char.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>This method always throws an <see cref="InvalidCastException"/>.</returns>
        char IConvertible.ToChar(IFormatProvider provider)
        {
            throw new InvalidCastException("Cannot cast BigDecimal to Char");
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a <see cref="DateTime"/>.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>This method always throws an <see cref="InvalidCastException"/>.</returns>
        DateTime IConvertible.ToDateTime(IFormatProvider provider)
        {
            throw new InvalidCastException("Cannot cast BigDecimal to DateTime");
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a decimal.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A decimal.</returns>
        decimal IConvertible.ToDecimal(IFormatProvider provider)
        {
            return Convert.ToDecimal(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a double.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A double.</returns>
        double IConvertible.ToDouble(IFormatProvider provider)
        {
            return Convert.ToDouble(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a short.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A short.</returns>
        short IConvertible.ToInt16(IFormatProvider provider)
        {
            return Convert.ToInt16(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to an int.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>An int.</returns>
        int IConvertible.ToInt32(IFormatProvider provider)
        {
            return Convert.ToInt32(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a long.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A long.</returns>
        long IConvertible.ToInt64(IFormatProvider provider)
        {
            return Convert.ToInt64(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a signed byte.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A signed byte.</returns>
        sbyte IConvertible.ToSByte(IFormatProvider provider)
        {
            return Convert.ToSByte(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a float.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A float.</returns>
        float IConvertible.ToSingle(IFormatProvider provider)
        {
            return Convert.ToSingle(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a string.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>A string.</returns>
        string IConvertible.ToString(IFormatProvider provider)
        {
            return Convert.ToString(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to an unsigned short.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>An unsigned short.</returns>
        ushort IConvertible.ToUInt16(IFormatProvider provider)
        {
            return Convert.ToUInt16(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to an unsigned int.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>An unsigned int.</returns>
        uint IConvertible.ToUInt32(IFormatProvider provider)
        {
            return Convert.ToUInt32(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to an unsigned long.
        /// </summary>
        /// <param name="provider">The format provider.</param>
        /// <returns>An unsigned long.</returns>
        ulong IConvertible.ToUInt64(IFormatProvider provider)
        {
            return Convert.ToUInt64(this, provider);
        }

        /// <summary>
        /// Converts the current <see cref="AIngleDecimal"/> to a string.
        /// </summary>
        /// <param name="format"></param>
        /// <param name="formatProvider">The format provider.</param>
        /// <returns>A string representation of the numeric value.</returns>
        public string ToString(string format, IFormatProvider formatProvider)
        {
            return ToString();
        }

        /// <summary>
        /// Compares the value of the current <see cref="AIngleDecimal"/> to the value of another object.
        /// </summary>
        /// <param name="obj">The object to compare.</param>
        /// <returns>A value that indicates the relative order of the objects being compared.</returns>
        public int CompareTo(object obj)
        {
            if (obj == null)
                return 1;

            if (!(obj is AIngleDecimal))
                throw new ArgumentException("Compare to object must be a BigDecimal", nameof(obj));

            return CompareTo((AIngleDecimal)obj);
        }

        /// <summary>
        /// Compares the value of the current <see cref="AIngleDecimal"/> to the value of another
        /// <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="other">The <see cref="AIngleDecimal"/> to compare.</param>
        /// <returns>A value that indicates the relative order of the <see cref="AIngleDecimal"/>
        /// instances being compared.</returns>
        public int CompareTo(AIngleDecimal other)
        {
            var unscaledValueCompare = UnscaledValue.CompareTo(other.UnscaledValue);
            var scaleCompare = Scale.CompareTo(other.Scale);

            // if both are the same value, return the value
            if (unscaledValueCompare == scaleCompare)
                return unscaledValueCompare;

            // if the scales are both the same return unscaled value
            if (scaleCompare == 0)
                return unscaledValueCompare;

            var scaledValue = BigInteger.Divide(UnscaledValue, BigInteger.Pow(new BigInteger(10), Scale));
            var otherScaledValue = BigInteger.Divide(other.UnscaledValue, BigInteger.Pow(new BigInteger(10), other.Scale));

            return scaledValue.CompareTo(otherScaledValue);
        }

        /// <summary>
        /// Returns a value that indicates whether the current <see cref="AIngleDecimal"/> has the same
        /// value as another <see cref="AIngleDecimal"/>.
        /// </summary>
        /// <param name="other">The <see cref="AIngleDecimal"/> to compare.</param>
        /// <returns>true if the current <see cref="AIngleDecimal"/> has the same value as <paramref name="other"/>;
        /// otherwise false.</returns>
        public bool Equals(AIngleDecimal other)
        {
            return Scale == other.Scale && UnscaledValue == other.UnscaledValue;
        }

        private static byte[] GetBytesFromDecimal(decimal d)
        {
            byte[] bytes = new byte[16];

            int[] bits = decimal.GetBits(d);
            int lo = bits[0];
            int mid = bits[1];
            int hi = bits[2];
            int flags = bits[3];

            bytes[0] = (byte)lo;
            bytes[1] = (byte)(lo >> 8);
            bytes[2] = (byte)(lo >> 0x10);
            bytes[3] = (byte)(lo >> 0x18);
            bytes[4] = (byte)mid;
            bytes[5] = (byte)(mid >> 8);
            bytes[6] = (byte)(mid >> 0x10);
            bytes[7] = (byte)(mid >> 0x18);
            bytes[8] = (byte)hi;
            bytes[9] = (byte)(hi >> 8);
            bytes[10] = (byte)(hi >> 0x10);
            bytes[11] = (byte)(hi >> 0x18);
            bytes[12] = (byte)flags;
            bytes[13] = (byte)(flags >> 8);
            bytes[14] = (byte)(flags >> 0x10);
            bytes[15] = (byte)(flags >> 0x18);

            return bytes;
        }
    }
    #pragma warning restore CA2225 // Operator overloads have named alternates
    #pragma warning restore CS1591 // Missing XML comment for publicly visible type or member
}
