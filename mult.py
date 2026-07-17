#!/usr/bin/env python3
import sys

# Multiply 5 * 4
a = 5
b = 4
if len(sys.argv) >= 3:
    a = int(sys.argv[1])
    b = int(sys.argv[2])
result = a * b
print("content-type: text/html")
print()
print(f"<html><body><h1>Result: {result}</h1></body></html>")