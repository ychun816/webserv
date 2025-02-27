#include "../../includes/server/Server.hpp"
// #include "Server.hpp"

// Constructors
Server::Server() : _configFile(""), _socketFd(-1) {}

Server::Server(const std::string & configFile) : _configFile(configFile), _socketFd(-1) {
	
}

// Destructor
Server::~Server() { std::cout << this->_configFile << std::endl; }

void	Server::createSocket() {
    if ((this->_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Erreur lors de la création du socket");
	std::memset(&this->address, 0, sizeof(this->address));
	this->address.sin_port = htons(8080);	// PORT
	this->address.sin_family = AF_INET;		// IPV4 INTERNET PROTOCOL
	this->address.sin_addr.s_addr = htonl(INADDR_ANY); // ADDRESS IP DE LA SOCKET (INADDR _ANY = l'os se debrouille)
	if (bind(this->_socketFd, (struct sockaddr *)&this->address, sizeof(this->address)) < 0) // Bind la socket a l'interface reseau, la relie. On peut donc s'y connecter en  utilisant le bon port et recevoir des connexions entrantes. En gros c'est son adresse
	{
		throw std::runtime_error("Bind failed");
	}
	else
		std::cout << "Bind Succes" << std::endl;
}

void	Server::configSocket() {
	// Sert a pouvoir reutiliser le port (ici 8080) et evite donc l'erreur d'adderss already in use
	const int enable = 1;
	if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
	
	// Pour un serveur web, une file d'attente de 10 à 128 connexions est généralement recommandée
	// SOMAXCONN est la valeur maximale recommandée par le système (généralement 128)
	if (listen(this->_socketFd, SOMAXCONN) < 0)
		throw std::runtime_error("Échec de l'écoute sur le socket");
	else
		std::cout << "Socket en écoute avec succès" << std::endl;
}

void Server::init(std::string &configFile) {
	this->_configFile = configFile;
	createSocket();
	configSocket();
}

// Operators
Server & Server::operator=(const Server &assign)
{
	(void) assign;
	return *this;
}

