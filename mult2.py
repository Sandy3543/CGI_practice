#!/usr/bin/env python3
import sys
import os

# Read POST body from standard input
body = sys.stdin.read()

print("Content-Type: text/html")
print("Status: 200 OK")
print()
print("<html><body>")
print(f"<h1>Received POST Body:</h1>")
print(f"<p>{body}</p>")
print("</body></html>")

sys.stdout.flush()

import time
time.sleep(100)