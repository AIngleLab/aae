#!/usr/bin/env python3

##
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# 
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import http.server
import sys
from typing import Mapping

import aingle.errors
import aingle.ipc
import aingle.protocol
import aingle.tether.tether_task
import aingle.tether.util

SERVER_ADDRESS = ("localhost", aingle.tether.util.find_port())


class MockParentResponder(aingle.ipc.Responder):
    """
    The responder for the mocked parent
    """

    def __init__(self) -> None:
        super().__init__(aingle.tether.tether_task.outputProtocol)

    def invoke(self, message: aingle.protocol.Message, request: Mapping[str, str]) -> None:
        response = f"MockParentResponder: Received '{message.name}'"
        responses = {
            "configure": f"{response}': inputPort={request.get('port')}",
            "status": f"{response}: message={request.get('message')}",
            "fail": f"{response}: message={request.get('message')}",
        }
        print(responses.get(message.name, response))
        sys.stdout.flush()  # flush the output so it shows up in the parent process


class MockParentHandler(http.server.BaseHTTPRequestHandler):
    """Create a handler for the parent."""

    def do_POST(self) -> None:
        self.responder = MockParentResponder()
        call_request_reader = aingle.ipc.FramedReader(self.rfile)
        call_request = call_request_reader.read_framed_message()
        resp_body = self.responder.respond(call_request)
        self.send_response(200)
        self.send_header("Content-Type", "aingle/binary")
        self.end_headers()
        resp_writer = aingle.ipc.FramedWriter(self.wfile)
        resp_writer.write_framed_message(resp_body)


def main() -> None:
    global SERVER_ADDRESS

    if len(sys.argv) != 3 or sys.argv[1].lower() != "start_server":
        raise aingle.errors.UsageError("Usage: mock_tether_parent start_server port")

    try:
        port = int(sys.argv[2])
    except ValueError as e:
        raise aingle.errors.UsageError("Usage: mock_tether_parent start_server port") from e

    SERVER_ADDRESS = (SERVER_ADDRESS[0], port)
    print(f"mock_tether_parent: Launching Server on Port: {SERVER_ADDRESS[1]}")

    # flush the output so it shows up in the parent process
    sys.stdout.flush()
    parent_server = http.server.HTTPServer(SERVER_ADDRESS, MockParentHandler)
    parent_server.allow_reuse_address = True
    parent_server.serve_forever()


if __name__ == "__main__":
    main()
