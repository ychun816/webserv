#include "../../includes/server/Server.hpp"
#include "../../includes/methods/Get.hpp" // added to execute methods
#include "../../includes/methods/Post.hpp" // added to execute methods
#include "../../includes/methods/Delete.hpp" // added to execute methods
#include <algorithm> // std::find
#include <arpa/inet.h>  // Ajout de cet en-tête pour inet_addr

// Constructors
Server::Server()
	: _epoll_fd(-1), _configFile(""), _socketFd(-1), _port(0), _host(""), _root(""), _index(""), _cgi(""), _upload(""),  _clientMaxBodySize("")
	,  _allowMethods(std::list<std::string>()), _errorPages(std::map<size_t, std::string>())
{
	_allowMethods.push_back("GET");
	_allowMethods.push_back("POST");
	_allowMethods.push_back("DELETE");
}

// Copy constructor
Server::Server(const Server& other) {
	_configFile = other._configFile;
	_socketFd = other._socketFd;
	_port = other._port;
	_host = other._host;
	_root = other._root;
	_index = other._index;
	_errorPages = other._errorPages;
	_cgi = other._cgi;
	_upload = other._upload;
	_clientMaxBodySize = other._clientMaxBodySize;
	_allowMethods = other._allowMethods;
	_connexions = other._connexions;
	_address = other._address;
	_errorPages = other._errorPages;
	_locations = other._locations;
	_return = other._return;
	_serverName = other._serverName;
}

// Destructor
Server::~Server() { std::cout << this->_configFile << std::endl; }


void    Server::createSocket() {
	if ((this->_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Socket creation, returns its FD
				throw std::runtime_error("Error while creating the socket");
		// Used to reuse the port  and thus avoid the "address already in use" error
		const int enable = 1;
		if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
				std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;

	// Allows multiple processes to bind to the same port
		if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
				std::cerr << "setsockopt(SO_REUSEPORT) failed" << std::endl;

		setNonBlocking(); // Switch to non-blocking mode
		std::memset(&this->_address, 0, sizeof(this->_address));
		// PORT
		this->_address.sin_port = htons(this->_port);
		// IPV4 INTERNET PROTOCOL
		this->_address.sin_family = AF_INET;
		// _address IP OF THE SOCKET (INADDR_ANY = the OS handles it)
		if (this->_host.empty()) {
			this->_address.sin_addr.s_addr = INADDR_ANY;
		} else {
			in_addr_t addr = inet_addr(this->_host.c_str());
			if (addr == INADDR_NONE) {
				this->_address.sin_addr.s_addr = INADDR_ANY;
			} else {
				this->_address.sin_addr.s_addr = addr;
			}
		}

		 // Bind the socket to the network interface, links it. It can then be connected to using the correct port and receive incoming connections. Essentially, it's its address
		if (bind(this->_socketFd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0)
		{
				close(this->_socketFd);
				throw Server::configError("Bind failed");
		}
		else
				std::cout << "Bind Success" << std::endl;
}

void    Server::configSocket() {
		// For a web server, a queue of 10 to 128 connections is generally recommended
		// SOMAXCONN is the maximum value recommended by the system (usually 128)
		if (listen(this->_socketFd, SOMAXCONN) < 0)
				throw Server::configError("Failed to listen on the socket");

}

void    Server::runServer() {
		// We will wait for a new incoming connection with accept(). Accept creates a new socket
		// specific to this connection and returns its FD

		struct  epoll_event event, events[MAX_EVENTS];

		// Create the epoll instance. The parameter is obsolete, so put whatever you want. Prefer Epoll_create1() but it's forbidden in the 42 webserv project
		if ((this->_epoll_fd = epoll_create(1)) < 0)
				throw Server::configError("Epoll Error");
		else
		{
				std::cout << "Epoll successfully created with fd: " << this->_epoll_fd << std::endl;
				event.events = EPOLLIN;
				event.data.fd = this->_socketFd;
				if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_socketFd, &event) == -1)
						throw Server::configError("Failed to add Socket to epoll (epoll_ctl)");
		}
		std::cout << "Waiting for connections on port " << this->_port << std::endl;
	while (true) {
		int event_count = epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
		if (event_count == -1) {
			throw Server::configError("epoll_wait");
			break;
		}
		for (int i = 0; i < event_count; i++) {
			if (events[i].data.fd == this->_socketFd) {
				handleNewConnection();
			} else {
				// Handling existing client connections
				try {
					std::string request = readChunkedData(events[i].data.fd);
					if (!request.empty()) {
						Request req(request, *this);
						req.handleResponse();

						// Envoi de la réponse en chunks si nécessaire
						std::string response = req.getResponse();
						size_t sent = 0;
						int chunk_count = 0;

						std::cout << "\n=== DEBUT ENVOI CHUNKS ===" << std::endl;
						std::cout << "Taille totale à envoyer: " << response.length() << " bytes" << std::endl;

						while (sent < response.length()) {
							chunk_count++;
							size_t toSend = std::min<size_t>(BUFFER_SIZE, response.length() - sent);

							std::cout << "CHUNK #" << chunk_count
									  << " | Taille: " << toSend
									  << " bytes | Progression: " << sent << "/"
									  << response.length() << " bytes" << std::endl;

							ssize_t bytes_sent = send(events[i].data.fd,
													response.c_str() + sent,
													toSend,
													0);
							if (bytes_sent <= 0) {
								std::cerr << "ERREUR: Échec envoi chunk" << std::endl;
								throw std::runtime_error("Error sending response");
							}
							sent += bytes_sent;
						}

						std::cout << "=== FIN ENVOI CHUNKS ===" << std::endl;
						std::cout << "Total chunks envoyés: " << chunk_count
								  << " | Taille totale: " << sent << " bytes\n" << std::endl;

						// Gestion de keep-alive
						if (req.getHeader("Connection") != "keep-alive") {
							close(events[i].data.fd);
							epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							removeConnexion(events[i].data.fd);
						}
					}
				} catch (const std::exception& e) {
					std::cerr << "Error handling request: " << e.what() << std::endl;
					close(events[i].data.fd);
					epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					removeConnexion(events[i].data.fd);
				}
			}
		}
	}
	close(this->_socketFd);
	close(this->_epoll_fd);
}

void Server::setNonBlocking()
{
		int flags = fcntl(this->_socketFd, F_GETFL, 0); // Retrieve the current flag of the socket
	if (flags == -1) {
		std::cerr << "Error fcntl(F_GETFL)" << std::endl;
	}
		std::cout << "Flag at socket creation: " << flags << std::endl;
	if (fcntl(this->_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) { // Add the O_NONBLOCK flag (becomes non-blocking)
		std::cerr << "Error fcntl(F_SETFL)" << std::endl;
	}
		std::cout << "Flag at socket creation: " << flags << std::endl;

}

void Server::pushLocation(const Location& location)
{
		this->_locations.push_back(location);
}

void Server::setEpollFd(int epoll_fd)
{
	this->_epoll_fd = epoll_fd;
}

void Server::handleNewConnection()
{
	struct sockaddr_in client_addr;
	socklen_t client_addrlen = sizeof(client_addr);
	int client_fd = accept(this->_socketFd, (struct sockaddr *)&client_addr, &client_addrlen);

	if (client_fd < 0) {
		std::cerr << "Accept failed" << std::endl;
		return;
	}

	std::cout << "New client connection accepted: " << client_fd << std::endl;
	this->_connexions.push_back(client_fd);

	// Add the client socket to epoll
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		std::cerr << "Error while adding the client to epoll" << std::endl;
		close(client_fd);
		this->_connexions.pop_back(); //removeConnexion(events[i].data.fd); ? use here too?
		return;
	}
}

void Server::addConnexion(int fd)
{
	this->_connexions.push_back(fd);
}

void Server::removeConnexion(int fd)
{
	std::deque<int>::iterator it = std::find(this->_connexions.begin(), this->_connexions.end(), fd);
	if (it != this->_connexions.end()) {
		this->_connexions.erase(it);
	}
}

// bool Server::errorPageExist(size_t code)
// {
//     return false;
// }

Server & Server::operator=(const Server &assign)
{
	if (this != &assign) {
		_configFile = assign._configFile;
		_socketFd = assign._socketFd;
		_port = assign._port;
		_host = assign._host;
		_root = assign._root;
		_index = assign._index;
		_errorPages = assign._errorPages;
		_cgi = assign._cgi;
		_upload = assign._upload;
		_clientMaxBodySize = assign._clientMaxBodySize;
		_allowMethods = assign._allowMethods;
		_connexions = assign._connexions;
		_address = assign._address;
		_locations = assign._locations;
		_return = assign._return;
		_serverName = assign._serverName;
	}
	return *this;
}


//ADDED
/**
 * @note cannot use switch case (use int) /  getMethod() return string X !
 * @note Get(). / Post(). / Delete(). -> temp create an object first to run exec()
 * @note *this -> current instance of that class (currently inside server class ) => passing the current Server instance by reference?? //TO CHECK!
 */

void Server::executeMethods(Request& request, Response& response)
{
	std::string method = request.getMethod();

	AMethods*       exec = NULL;
	
	//ADDED TO GET UPLOAD DIRECTORY ///SAVED HERE FOR FUTRE USE
	// if (request.getMethod() == "GET" && request.getUploadDirectory(request.getUri()) != "")
	// {
	// 	response.setStatus(200);
	// 	// response.setHeaders("Content-Type", "application/json");
	// 	response.setHeaders(request.getHeaders());
	// 	// response.setHttpVersion(request.getHttpVersion());

	// 	std::string uploadDir = request.getUploadDirectory(request.getUri());
	// 	response.setBody("{" + uploadDir + "\"}");

	// 	std::cout << "🍓SEVER | Upload directory: " << uploadDir << std::endl; //DEBUG ////
	// 	return;
	// }

	//general methods
	if (method == "GET")
	{
		std::cout << "SEVER-GET" << std::endl;
		exec = new Get();
	}
	else if (method == "POST")
	{
		std::cout << "SEVER-POST" << std::endl;
		exec = new Post();
	}

	else if (method == "DELETE")
		exec = new Delete();
	else
	{
		response.setStatus(405); //method not allowed
	}
	if (exec)
	{
		exec->process(request, response, *this);
		delete exec; // Free the memory allocated for the method?
	}
}

Location* Server::getCurrentLocation(const std::string& path) {
    std::list<Location>::iterator it = _locations.begin();
    Location* bestMatch = NULL;
    size_t bestMatchLength = 0;

    for (; it != _locations.end(); ++it) {
        std::string locationPath = it->getPath();
        // Vérifie si le chemin de la requête commence par le chemin de la location
        if (path.find(locationPath) == 0) {
            // Garde la location avec le chemin le plus long qui correspond
            if (locationPath.length() > bestMatchLength) {
                bestMatch = &(*it);
                bestMatchLength = locationPath.length();
            }
        }
    }
    return bestMatch;
}

bool Server::isServerNameMatch(const std::string& hostHeader) const {
    if (_host.empty()) return true;  // Si pas de server_name configuré
    
    // Extraire le nom d'hôte sans le port
    std::string host = hostHeader;
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        host = host.substr(0, colonPos);
    }
    
    // Vérification stricte
    return host == _host;
}

/* TO FIX?
2. Incorrect _connexions.pop_back() Use

You are calling pop_back() regardless of which client FD is being closed. This will remove the wrong connection in most cases.

Fix: Use removeConnexion(fd); instead:

close(events[i].data.fd);
epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
removeConnexion(events[i].data.fd); // FIXED


*/