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
	setNonBlocking(); // Passage en mode non-Bloquant
	std::memset(&this->address, 0, sizeof(this->address));
	// PORT
	this->address.sin_port = htons(PORT);
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

	struct	epoll_event event, events[MAX_EVENTS];
	int		client_fd, epoll_fd;

	// Creer l'instance epoll. le parametre est obsolete donc mettre ce qu'on veut. Preferer Epoll_create1() mais interdit dan sle projet 42 webserv
	if ((epoll_fd = epoll_create(1)) < 0)
		throw std::runtime_error("Epoll Error");
	else
	{
		std::cout << "Epoll crée avec succes with fd: " << epoll_fd << std::endl;
		event.events = EPOLLIN;
		event.data.fd = this->_socketFd;
 		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->_socketFd, &event) == -1)
			throw std::runtime_error("Fail to add Socket to epoll (epoll_ctl)");
	}
	std::cout << "En attente de connexions sur le port " << PORT << std::endl;
    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            throw std::runtime_error("epoll_wait");
            break;
        }
		for (int i = 0; i < event_count; i++)
		{
			if (events[i].data.fd == this->_socketFd) { // Nouvelle connexion
				struct sockaddr_in client_address;
				socklen_t client_addrlen = sizeof(client_address);
				client_fd = accept(this->_socketFd, (struct sockaddr *)&client_address, &client_addrlen);
				if (client_fd < 0) {
					std::cerr << "Accept echoué" << std::endl;
					continue;
				}
			std::cout << "Nouvelle connexion client acceptée: " << client_fd << std::endl;
			this->_connexions.push_back(client_fd);
            event.events = EPOLLIN;
            event.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
			}
			else
			{
				// Lecture client
				char buffer[1024] = {0};
				int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
				if (bytes_read > 0)
				{
					std::cout << "Bytes read: " << buffer << std::endl;
					send(events[i].data.fd, buffer, bytes_read, 0);

				} else {
					// deconnexion client
					std::cout << "client deconnecté: " << events[i].data.fd << std::endl;
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);

				}
			}
			
		}
	}
	close(this->_socketFd);
	close(epoll_fd);
}

void Server::setNonBlocking()
{
	int flags = fcntl(this->_socketFd, F_GETFL, 0); // Recupere  le flag actuel de la socket
    if (flags == -1) {
        std::cerr << "Erreur fcntl(F_GETFL)" << std::endl;
    }
	std::cout << "Flag a la creation de la socket: " << flags << std::endl;
    if (fcntl(this->_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) { // Ajoute le flag 0_NONBLOCK (devient non bloquant)
        std::cerr << "Erreur fcntl(F_SETFL)" << std::endl;
    }
	std::cout << "Flag a la creation de la socket: " << flags << std::endl;

}
void Server::init(std::string &configFile)
{
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

