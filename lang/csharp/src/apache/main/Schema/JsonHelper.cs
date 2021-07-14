/*

 */
using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;

namespace AIngle
{
    class JsonHelper
    {
        /// <summary>
        /// Retrieves the optional string property value for the given property name from the given JSON object.
        /// This throws an exception if property exists but it is not a string.
        /// </summary>
        /// <param name="jtok">JSON object to read</param>
        /// <param name="field">property name</param>
        /// <returns>property value if property exists, null if property doesn't exist in the JSON object</returns>
        public static string GetOptionalString(JToken jtok, string field)
        {
            if (null == jtok) throw new ArgumentNullException(nameof(jtok), "jtok cannot be null.");
            if (string.IsNullOrEmpty(field)) throw new ArgumentNullException(nameof(field), $"field cannot be null at '{jtok.Path}'");

            JToken child = jtok[field];
            if (null == child) return null;

            if (child.Type == JTokenType.String)
            {
                return child.Value<string>();
            }
            throw new SchemaParseException($"Field {field} is not a string at '{jtok.Path}'");
        }

        /// <summary>
        /// Retrieves the required string property value for the given property name from the given JSON object.
        /// </summary>
        /// <param name="jtok">JSON object to read</param>
        /// <param name="field">property name</param>
        /// <returns>property value</returns>
        public static string GetRequiredString(JToken jtok, string field)
        {
            string value = GetOptionalString(jtok, field);
            if (string.IsNullOrEmpty(value)) throw new SchemaParseException($"No \"{field}\" JSON field: {Regex.Replace(jtok.ToString(), @"\r\n?|\n", "")} at '{jtok.Path}'");
            return value;
        }

        /// <summary>
        /// Retrieves the required int property value for the given property name from the given JSON object.
        /// </summary>
        /// <param name="jtok">JSON object to read</param>
        /// <param name="field">property name</param>
        /// <returns>property value</returns>
        public static int GetRequiredInteger(JToken jtok, string field)
        {
            if (string.IsNullOrEmpty(field)) throw new ArgumentNullException(nameof(field));

            JToken child = jtok[field];
            if (null == child) throw new SchemaParseException($"No \"{field}\" JSON field: {Regex.Replace(jtok.ToString(), @"\r\n?|\n", "")} at '{jtok.Path}'");

            if (child.Type == JTokenType.Integer) return (int) child;
            throw new SchemaParseException($"Field {field} is not an integer at '{jtok.Path}'");
        }

        /// <summary>
        /// Retrieves the optional boolean property value for the given property name from the given JSON object.
        /// </summary>
        /// <param name="jtok">JSON object to read</param>
        /// <param name="field">property name</param>
        /// <returns>null if property doesn't exist, otherise returns property boolean value</returns>
        public static bool? GetOptionalBoolean(JToken jtok, string field)
        {
            if (null == jtok) throw new ArgumentNullException(nameof(jtok), "jtok cannot be null.");
            if (string.IsNullOrEmpty(field)) throw new ArgumentNullException(nameof(field), $"field cannot be null at '{jtok.Path}'");

            JToken child = jtok[field];
            if (null == child) return null;

            if (child.Type == JTokenType.Boolean)
                return (bool)child;

            throw new SchemaParseException($"Field {field} is not a boolean at '{jtok.Path}'");
        }

        /// <summary>
        /// Writes JSON property name and value if value is not null
        /// </summary>
        /// <param name="writer">JSON writer</param>
        /// <param name="key">property name</param>
        /// <param name="value">property value</param>
        internal static void writeIfNotNullOrEmpty(JsonTextWriter writer, string key, string value)
        {
            if (string.IsNullOrEmpty(value)) return;
            writer.WritePropertyName(key);
            writer.WriteValue(value);
        }

    }
}
