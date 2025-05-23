#!/usr/bin/env python3
# This is a basic CGI script that returns a simple HTML page
print("Content-Type: text/html\n")  # Header specifying the content type
print("<html><body><h1>Hello from Python CGI!</h1></body></html>")  # Output HTML content


###########
#To Test:
#Make a GET request to this file 
#(e.g., http://localhost/cgi-bin/hello.py) 
#and check if the server processes the Python script and returns the correct HTML response.