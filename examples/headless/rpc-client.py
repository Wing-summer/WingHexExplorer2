# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-only
"""Minimal Content-Length JSON-RPC client / 简单的长度帧 JSON-RPC 客户端。"""
import argparse
import json
import socket
import ssl

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("method", help="task.execute, task.list, task.stop, service.shutdown, ...")
parser.add_argument("params", nargs="?", default="{}", help="JSON named parameters")
parser.add_argument("--address", default="127.0.0.1")
parser.add_argument("--port", type=int, default=18888)
parser.add_argument("--ca", help="Trusted server certificate or CA PEM file")
parser.add_argument("--cert", help="Registered client certificate PEM file")
parser.add_argument("--key", help="Client private key PEM file")
parser.add_argument("--watch", action="store_true", help="Read subscribed task notifications until disconnected")
args = parser.parse_args()
connection = socket.create_connection((args.address, args.port))
if args.ca:
    context = ssl.create_default_context(cafile=args.ca)
    if args.cert:
        context.load_cert_chain(args.cert, args.key)
    connection = context.wrap_socket(connection, server_hostname=args.address)
with connection:
    body = json.dumps(dict(jsonrpc="2.0", id=1, method=args.method, params=json.loads(args.params)),
                      ensure_ascii=False).encode("utf-8")
    connection.sendall(f"Content-Length: {len(body)}\r\n\r\n".encode("ascii") + body)
    stream = connection.makefile("rb")
    while True:
        length = None
        while line := stream.readline():
            if line == b"\r\n":
                break
            name, value = line.split(b":", 1)
            if name.lower() == b"content-length":
                length = int(value)
        if length is None:
            break
        response = json.loads(stream.read(length))
        print(json.dumps(response, ensure_ascii=False), flush=True)
        if not args.watch and "id" in response:
            break
