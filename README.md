# Webserv - Serveur HTTP en C++98 ♫⋆｡♪ [![Spotify](https://img.shields.io/badge/Listen_on-Spotify-1DB954?style=for-the-badge&logo=spotify)](https://open.spotify.com/track/69kOkLUCkxIZYexIgSG8rq)

![Screenshot from 2025-06-11 16-30-04](https://github.com/user-attachments/assets/4839e8e7-7f47-4046-b392-467a044e9a60)


## 📝 Description
Implémentation d'un serveur HTTP en C++98 capable de gérer des requêtes web de manière non bloquante, similaire à NGINX.

## 🛠️ Prérequis
- Compilateur C++ avec support C++98
- Make
- Système Unix-like (Linux/MacOS)

## 🚀 Installation
```bash
git clone [votre-repo]
cd webserv
make
```

## 📂 Structure du Projet
```
webserv/
├── srcs/           # Fichiers sources
├── includes/       # Headers
├── config/         # Fichiers de configuration
├── www/           # Contenu web statique pour les tests
├── Makefile
└── README.md
```

## 🎯 Fonctionnalités Principales
- Serveur HTTP non-bloquant
- Support des méthodes GET, POST, DELETE
- Configuration personnalisable (ports, routes, etc.)
- Gestion des fichiers statiques
- Support CGI (PHP, Python)
- Gestion des uploads de fichiers
- Pages d'erreur personnalisables

## 🔧 Configuration
Le serveur peut être configuré via un fichier de configuration. Exemple:
```nginx
server {
    listen 8080;
    server_name example.com;
    root /var/www/html;
    
    location / {
        methods GET POST;
        index index.html;
        autoindex on;
    }
}
```

## 📋 Étapes de Développement

### 1. Base du Serveur
- [x] Création du Makefile
- [x] Configuration de base du serveur
- [x] Implémentation du socket non-bloquant
- [x] Gestion basique des connexions

### 2. Parsing HTTP
- [x] Parser de requêtes HTTP
- [x] Générateur de réponses HTTP
- [x] Gestion des headers

### 3. Fonctionnalités Core
- [x] Gestion des méthodes HTTP (GET, POST, DELETE)
- [x] Gestion des fichiers statiques
- [x] Configuration dynamique
- [x] Pages d'erreur

### 4. Fonctionnalités Avancées
- [x] Support CGI
- [x] Upload de fichiers
- [x] Directory listing
- [x] Redirections

## 📚 Documentation Utile
- [RFC 2616 (HTTP/1.1)](https://tools.ietf.org/html/rfc2616)
- [Documentation NGINX](https://nginx.org/en/docs/)
- [CGI Specification](https://datatracker.ietf.org/doc/html/rfc3875)
- [NGINX Begginner Guide](https://nginx.org/en/docs/beginners_guide.html)

## ⚠️ Notes Importantes
- Le serveur doit rester disponible sous forte charge
- Toutes les opérations I/O doivent être non-bloquantes
- La gestion des erreurs doit être robuste
- Le code doit être conforme à la norme C++98

## 🤝 Contribution
Ce projet est développé par:
- [agilibert](https://github.com/Canybardeloton)
- [clarily](https://github.com/CrystxlSith)
- [yilin](https://github.com/ychun816)

## 📄 Licence
42

dd if=/dev/zero of=large_file.txt bs=1M count=10
curl -v -o downloaded_file.txt http://localhost:8080/upload/large_file.txt

test sans methode : echo -e " / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080
stress test siege :  siege -b -c 255 -t1M http://127.0.0.1:8080/

---

## 🧪 TEST COMMANDS (WITH EXPLAINS)

### Pour tester le serveur (To launch):
```bash
# Démarrer le serveur
./webserv [config_file]

# Test basique avec curl
curl http://localhost:8080

# Test avec un navigateur
# Ouvrez http://localhost:8080 dans votre navigateur
```
---

### 🌐 Basic HTTP Request Tests

* **Raw HTTP Request via Netcat**

  ```bash
  echo -e "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080
  ```

  Sends a minimal HTTP GET request manually to inspect raw response behavior.

* **Basic Netcat Connection**

  ```bash
  nc localhost 8080
  ```

  Establishes a plain TCP connection to test server responsiveness.

* **Netcat with Timeout**

  ```bash
  nc -i 5 localhost 8080
  ```

  Similar to the above, with a 5-second interval between lines to simulate slow clients.

---

### 🧷 POST and DELETE Method Tests

* **POST Request with Form Data**

  ```bash
  curl -X POST -d "username=test&password=123" http://localhost:8080/cgi-bin/post.py
  ```

  Tests server's ability to handle POST requests and form data processing.

* **DELETE Request Simulation**

  ```bash
  curl -X DELETE http://localhost:8080/chemin/vers/ressource
  ```

  Verifies support for the DELETE HTTP method, often used in RESTful APIs.

---

### 📁 File Creation & Download

* **Generate a Large Test File**

  ```bash
  dd if=/dev/zero of=large_file.txt bs=11M count=10
  ```

  Creates a 110MB dummy file for upload and download testing.

* **Download File Using `curl`**

  ```bash
  curl -v -o downloaded_file.txt http://localhost:8080/upload/large_file.txt
  ```

  Verifies file downloading capability from the server.

---

### 🔐 HTTP Protocol Variants & Headers

* **Force HTTP/1.0 with Connection Close**

  ```bash
  curl -v --http1.0 -H "Connection: close" http://localhost:8080/
  ```

  Checks server compatibility with legacy HTTP/1.0 and explicit connection closure.

* **Custom Host Header**

  ```bash
  curl -v -H "Host: getlucky" http://127.0.0.1:8080/
  ```

  Tests virtual hosting by specifying a non-default Host header.

---

### 🔁 Load & Stress Testing

* **Siege Stress Test**

  ```bash
  siege -b -c 255 -t1M http://127.0.0.1:8080/
  ```

  Runs a high-concurrency benchmark (255 clients for 1 minute) to assess server performance under load.

---

### 📚 Reference

* **Nginx Server Name Configuration**

  * [Nginx Server Names Documentation](https://nginx.org/en/docs/http/server_names.html)
    Useful for configuring and understanding virtual hosting.





