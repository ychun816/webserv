#!/usr/bin/env python3
# This is a basic CGI script that returns a simple HTML page
import os

print("Content-Type: text/html\n")  # Header specifying the content type
print("Set-Cookie: session=abc123")
print("<html><body><h1>Hello from Python CGI!</h1></body></html>")  # Output HTML content
