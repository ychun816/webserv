# Webserv - Serveur HTTP en C++98
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
- [ ] Gestion des méthodes HTTP (GET, POST, DELETE)
- [ ] Gestion des fichiers statiques
- [ ] Configuration dynamique
- [ ] Pages d'erreur

### 4. Fonctionnalités Avancées
- [ ] Support CGI
- [ ] Upload de fichiers
- [ ] Directory listing
- [ ] Redirections

## 🧪 Tests
Pour tester le serveur:
```bash
# Démarrer le serveur
./webserv [config_file]

# Test basique avec curl
curl http://localhost:8080

# Test avec un navigateur
# Ouvrez http://localhost:8080 dans votre navigateur
```

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
