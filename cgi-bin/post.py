#!/usr/bin/env python3
import os
import sys

# Force unbuffered output
sys.stdout.flush()

print("Content-Type: text/html\r")
print("\r")
sys.stdout.flush()

print("<html><body>")
print("<h1>POST Data Received</h1>")
sys.stdout.flush()

# Print environment variables
print("<h2>Environment Variables:</h2>")
for key in ['REQUEST_METHOD', 'CONTENT_TYPE', 'CONTENT_LENGTH', 'QUERY_STRING']:
    print(f"<p>{key}: {os.environ.get(key, 'Not set')}</p>")
sys.stdout.flush()

# Read POST data
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length and content_length != '':
        try:
            content_length = int(content_length)
            if content_length > 0:
                post_data = sys.stdin.read(content_length)
                print(f"<h2>POST Data:</h2><pre>{post_data}</pre>")
                sys.stdout.flush()
        except ValueError:
            print("<p>Invalid Content-Length</p>")
            sys.stdout.flush()
    else:
        print("<p>No Content-Length header</p>")
        sys.stdout.flush()

print("</body></html>")
sys.stdout.flush()