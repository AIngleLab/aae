/*

 */
namespace AIngle.Specific
{
    /// <summary>
    /// Defines the interface for a class that implements a specific protocol.
    /// TODO: This interface needs better documentation.
    /// </summary>
    public interface ISpecificProtocol
    {
        /// <summary>
        /// Protocol for this instance.
        /// </summary>
        Protocol Protocol { get; }

        /// <summary>
        /// Execute a request.
        /// </summary>
        /// <param name="requestor">Callback requestor.</param>
        /// <param name="messageName">Name of the message.</param>
        /// <param name="args">Arguments for the message.</param>
        /// <param name="callback">Callback.</param>
        void Request(ICallbackRequestor requestor, string messageName, object[] args, object callback);
    }

    /// <summary>
    /// TODO: This interface needs better documentation.
    /// </summary>
    public interface ICallbackRequestor
    {
        /// <summary>
        /// Request
        /// </summary>
        /// <typeparam name="T">Type</typeparam>
        /// <param name="messageName">Name of the message.</param>
        /// <param name="args">Arguments for the message.</param>
        /// <param name="callback">Callback.</param>
        void Request<T>(string messageName, object[] args, object callback);
    }

}
