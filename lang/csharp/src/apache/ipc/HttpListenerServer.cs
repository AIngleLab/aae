/**

 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Diagnostics;

namespace AIngle.ipc
{
    public class HttpListenerServer
    {
        IEnumerable<string> _prefixes;
        HttpListener _listener;
        Responder _responder;

        public HttpListenerServer(IEnumerable<string> listenOnPrefixes, Responder responder)
        {
            _responder = responder;
            _prefixes = listenOnPrefixes;
        }

        //TODO: apparently this doesn't compile in Mono - investigate
        //public Action<Exception, IAsyncResult> ExceptionHandler { get; set; }

        protected void HttpListenerCallback(IAsyncResult result)
        {
            try
            {
                HttpListener listener = (HttpListener)result.AsyncState;
                if (_listener != listener) //the server which began this callback was stopped - just exit
                    return;
                HttpListenerContext context = listener.EndGetContext(result);

                listener.BeginGetContext(HttpListenerCallback, listener); //spawn listening for next request so it can be processed while we are dealing with this one

                //process this request
                if (!context.Request.HttpMethod.Equals("POST"))
                    throw new AIngleRuntimeException("HTTP method must be POST");
                if (!context.Request.ContentType.Equals("aingle/binary"))
                    throw new AIngleRuntimeException("Content-type must be aingle/binary");

                byte[] intBuffer = new byte[4];
                var buffers = HttpTransceiver.ReadBuffers(context.Request.InputStream, intBuffer);

                buffers = _responder.Respond(buffers);
                context.Response.ContentType = "aingle/binary";
                context.Response.ContentLength64 = HttpTransceiver.CalculateLength(buffers);

                HttpTransceiver.WriteBuffers(buffers, context.Response.OutputStream);

                context.Response.OutputStream.Close();
                context.Response.Close();
            }
            catch (Exception ex)
            {
                //TODO: apparently this doesn't compile in Mono - investigate
                //if (ExceptionHandler != null)
                //    ExceptionHandler(ex, result);
                //else
                //    Debug.Print("Exception occured while processing a request, no exception handler was provided - ignoring", ex);
                Debug.Print("Exception occured while processing a web request, skipping this request: ", ex);
            }
        }

        public void Start()
        {
            _listener = new HttpListener();

            foreach (string s in _prefixes)
            {
                _listener.Prefixes.Add(s);
            }

            _listener.Start();

            _listener.BeginGetContext(HttpListenerCallback, _listener);
        }

        public void Stop()
        {
            _listener.Stop();
            _listener.Close();
            _listener = null;
        }
    }
}
