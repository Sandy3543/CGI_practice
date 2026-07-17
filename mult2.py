#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

# Grab QUERY_STRING that C++ passes via envp
query_string = os.environ.get("QUERY_STRING", "")

# Parse "a=3&b=5"
parsed = parse_qs(query_string)

a = int(parsed.get('a', [1])[0])
b = int(parsed.get('b', [1])[0])

result = a * b

print("content-type: text/html")
print()
print(f"<html><body><h1>Result: {result}</h1></body></html>")