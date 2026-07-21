#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

# Grab QUERY_STRING that C++ passes via envp
query_string = os.environ.get("QUERY_STRING", "")
script_name = os.environ.get("SCRIPT_NAME", "")
script_filename = os.environ.get("SCRIPT_FILENAME", "")
server_name = os.environ.get("SERVER_NAME", "")
server_port = os.environ.get("SERVER_PORT", "")
server_protocol = os.environ.get("SERVER_PROTOCOL", "")
server_software = os.environ.get("SERVER_SOFTWARE", "")
request_method = os.environ.get("REQUEST_METHOD", "")
request_uri = os.environ.get("REQUEST_URI", "")
gateway_interface = os.environ.get("GATEWAY_INTERFACE", "")
path_info = os.environ.get("PATH_INFO", "")
path_translated = os.environ.get("PATH_TRANSLATED", "")
redirect_status = os.environ.get("REDIRECT_STATUS", "")

# Parse "a=3&b=5"
parsed = parse_qs(query_string)

a = int(parsed.get('a', [1])[0])
b = int(parsed.get('b', [1])[0])

result = a * b

print("content-type: text/html")
print("Status: 302 OK")
print()
print(f"<html><body><h1>Result: {result}</h1></body></html>")