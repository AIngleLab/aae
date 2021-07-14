/*

 */

using System;

namespace AIngle.IO
{
    /// <summary>
    /// Obsolete - This will be removed from the public API in a future version.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [Obsolete("This will be removed from the public API in a future version.")]
    public interface ICallback<in T>
    {
        /// <summary>
        /// Receives a callback result.
        /// </summary>
        /// <param name="result">Result returned in the callback.</param>
        void HandleResult(T result);

        /// <summary>
        /// Receives an error.
        /// </summary>
        /// <param name="exception">Error returned in the callback.</param>
        void HandleException(Exception exception);
    }
}
