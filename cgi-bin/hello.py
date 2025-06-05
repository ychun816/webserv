#!/usr/bin/env python3
print("Content-Type: text/html\r\n\r\n")

print("""
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Hello from Python CGI</title>
  <link href="https://fonts.googleapis.com/css2?family=Josefin+Sans:wght@300;400;700&display=swap" rel="stylesheet" />
  <style>
    body {
      margin: 0;
      font-family: 'Josefin Sans', sans-serif;
      background: url('https://4kwallpapers.com/images/wallpapers/rosette-nebula-10351.jpg') no-repeat center center fixed;
      background-size: cover;
      color: white;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      text-align: center;
    }

    .message {
      background-color: rgba(0, 0, 0, 0.6);
      padding: 2rem 3rem;
      border-radius: 15px;
      box-shadow: 0 0 15px rgba(255, 255, 255, 0.2);
    }

    h1 {
      font-size: 2.5em;
      color: #ffcdd8;
      text-shadow: 0 0 10px #ff8696;
    }

    .button {
      margin-top: 1.5rem;
      background-color: #ff8696;
      border: none;
      padding: 0.75rem 1.5rem;
      border-radius: 10px;
      color: white;
      font-size: 1rem;
      cursor: pointer;
      text-decoration: none;
      display: inline-block;
      transition: background-color 0.3s ease;
    }

    .button:hover {
      background-color: #ff4d6d;
    }
  </style>
</head>
<body>
  <div class="message">
    <h1>Hello from galaxy!</h1>
    <a class="button" href="/index.html">Return to Index</a>
  </div>
</body>
</html>
""")
