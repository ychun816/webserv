#include "../../includes/server/Server.hpp"
// #include "Server.hpp"

// Constructors
Server::Server() : _configFile(""), _socketFd(-1) {}

Server::Server(const std::string & configFile) : _configFile(configFile), _socketFd(-1) {
	
}

// Destructor
Server::~Server() { std::cout << this->_configFile << std::endl; }

void	Server::createSocket() {
    if ((this->_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Creation de la socket, retourne son FD
		throw std::runtime_error("Erreur lors de la création du socket");
	int flags = fcntl(this->_socketFd, F_GETFL, 0); // Recupere  le flag actuel de la socket
    if (flags == -1) {
        std::cerr << "Erreur fcntl(F_GETFL)" << std::endl;
    }
    if (fcntl(this->_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) { // Ajoute le flag 0_NONBLOCK (devient non bloquant)
        std::cerr << "Erreur fcntl(F_SETFL)" << std::endl;
    }
	std::cout << "Flag a la creation de la socket: " << flags << std::endl;
	std::memset(&this->address, 0, sizeof(this->address));
	// PORT
	this->address.sin_port = htons(8080);
	// IPV4 INTERNET PROTOCOL
	this->address.sin_family = AF_INET;
	// ADDRESS IP DE LA SOCKET (INADDR _ANY = l'os se debrouille)
	this->address.sin_addr.s_addr = htonl(INADDR_ANY);
	 // Bind la socket a l'interface reseau, la relie. On peut donc s'y connecter en  utilisant le bon port et recevoir des connexions entrantes. En gros c'est son adresse
	if (bind(this->_socketFd, (struct sockaddr *)&this->address, sizeof(this->address)) < 0)
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

}

void	Server::runServer() {
	// On va attendre une nouvelle connexion entrante avec accept(). acept cree un nouveau socket
	// specifique a cette connexion et return son FD
	std::cout << "En attente de connexions..." << std::endl;
	socklen_t addrlen = sizeof(this->address);
	int		new_socket;

	while (true)
	{
		new_socket = accept(this->_socketFd, (struct sockaddr *)&this->address, &addrlen);
		if (new_socket < 0) {
			std::cerr << "Accept eechoué" << std::endl;
			continue;
		}
		std::cout << "Nouvelle connexion acceptée: " << new_socket << std::endl;
		this->_connexions.push_back(new_socket);
	}
	close(this->_socketFd);
	
}

void Server::init(std::string &configFile) {
	this->_configFile = configFile;
	createSocket();
	configSocket();
	runServer();
}

// Operators
Server & Server::operator=(const Server &assign)
{
	(void) assign;
	return *this;
}

