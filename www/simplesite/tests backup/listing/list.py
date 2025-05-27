#!/usr/bin/env python3
import os
import urllib.parse

print("Content-Type: text/html\n")
print("""
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<title>Directory Listing - Webserv</title>
<link href="https://fonts.googleapis.com/css2?family=Josefin+Sans:wght@300;400;700&display=swap" rel="stylesheet" />
<style>
  /* (Paste the same CSS styles here from the static list.html for consistency) */
  *{margin:0;padding:0;box-sizing:border-box;}
  html,body{height:100%;font-family:'Josefin Sans',sans-serif;background:#000;color:#fff;overflow-y:auto;padding:2rem;}
  body::before{content:"";position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.7);z-index:0;}
  .container{position:relative;z-index:1;max-width:800px;margin:0 auto;background:rgba(255,255,255,0.1);border-radius:15px;padding:2rem;box-shadow:0 0 20px rgba(255,255,255,0.2);}
  h1{text-align:center;margin-bottom:1.5rem;text-shadow:0 0 10px #ffcdd8,0 0 20px #ff8696;}
  ul{list-style:none;}
  li{margin:0.7rem 0;font-size:1.2rem;padding:0.5rem 1rem;background:rgba(255,255,255,0.15);border-radius:8px;transition:background-color 0.3s;}
  li a{color:#bbdbf8;text-decoration:none;display:block;}
  li a:hover{background:#fdd281;color:#000;border-radius:8px;}
  .nav-links{margin-top:2rem;display:flex;justify-content:center;gap:1.5rem;}
  .nav-links a{text-decoration:none;color:#bbdbf8;font-size:1rem;padding:0.5rem 1.2rem;border:1px solid #bbdbf8;border-radius:8px;background:rgba(0,0,0,0.3);transition:all 0.3s;}
  .nav-links a:hover{background:#fdd281;color:#000;border-color:#fdd281;}
</style>
</head>
<body>
<div class="container">
<h1>📂 Directory Listing</h1>
<ul>
""")

# Get the current directory path
current_dir = os.getcwd()

# Optional: get path from query string for subdirectories
import cgi
form = cgi.FieldStorage()
path = form.getvalue("path", ".")

# Sanitize the path to avoid directory traversal
path = os.path.normpath(path)
if path.startswith('..'):
    path = '.'

try:
    files = os.listdir(path)
except Exception as e:
    print(f"<p style='color:red;'>Error reading directory: {e}</p>")
    files = []

# Sort directories first, then files
files.sort(key=lambda f: (not os.path.isdir(os.path.join(path, f)), f.lower()))

# Parent directory link (if not root)
if path != '.':
    parent_path = os.path.dirname(path)
    print(f'<li><a href="?path={urllib.parse.quote(parent_path)}">../ (Parent Directory)</a></li>')

for f in files:
    full_path = os.path.join(path, f)
    display_name = f + '/' if os.path.isdir(full_path) else f
    link_path = urllib.parse.quote(os.path.join(path, f))
    print(f'<li><a href="?path={link_path}">{display_name}</a></li>')

print("""
</ul>
<div class="nav-links">
  <a href="/">🏠 Root</a>
  <a href="/upload/">📤 Upload</a>
  <a href="/upload/download.html">📥 Download</a>
  <a href="/delete.html">🗑️ Delete</a>
</div>
</div>
</body>
</html>
""")
