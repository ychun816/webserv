#include "../../includes/server/Server.hpp"
#include "../../includes/methods/Get.hpp" // added to execute methods
#include "../../includes/methods/Post.hpp" // added to execute methods
#include "../../includes/methods/Delete.hpp" // added to execute methods
#include <algorithm> // std::find

#define BUFFER_SIZE 8192  // 8KB
#define MAX_REQUEST_SIZE (10 * 1024 * 1024)  // 10MB

// Constructors
Server::Server() : _epoll_fd(-1), _configFile(""), _socketFd(-1), _port(0), _host(""), _root(""), _index(""), _errorPage(""), _cgi(""), _upload(""), _clientMaxBodySize(""), _allowMethods(std::list<std::string>()) {


}

// Copy constructor
Server::Server(const Server& other) {
	_configFile = other._configFile;
	_socketFd = other._socketFd;
	_port = other._port;
	_host = other._host;
	_root = other._root;
	_index = other._index;
	_errorPage = other._errorPage;
	_cgi = other._cgi;
	_upload = other._upload;
	_clientMaxBodySize = other._clientMaxBodySize;
	_allowMethods = other._allowMethods;
	_connexions = other._connexions;
	_address = other._address;
	_errorPages = other._errorPages;
	_locations = other._locations;
}

// Destructor
Server::~Server() { std::cout << this->_configFile << std::endl; }

std::string readChunkedData(int client_fd) {
    std::string completeData;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    size_t totalBytesRead = 0;
    int chunk_count = 0;

    std::cout << "\n=== DEBUT LECTURE CHUNKS ===" << std::endl;
    std::cout << "Client " << client_fd << " - Début lecture" << std::endl;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        chunk_count++;
        completeData.append(buffer, bytes_read);
        totalBytesRead += bytes_read;

        std::cout << "CHUNK #" << chunk_count
                  << " | Taille: " << bytes_read
                  << " bytes | Total: " << totalBytesRead
                  << " bytes" << std::endl;

        if (totalBytesRead > MAX_REQUEST_SIZE) {
            std::cerr << "ERREUR: Requête trop grande: " << totalBytesRead << " bytes" << std::endl;
            throw std::runtime_error("Request too large");
        }
    }

    std::cout << "=== FIN LECTURE CHUNKS ===" << std::endl;
    std::cout << "Client " << client_fd
              << " | Chunks: " << chunk_count
              << " | Taille totale: " << totalBytesRead
              << " bytes\n" << std::endl;

    return completeData;
}
void    Server::createSocket() {
	if ((this->_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Socket creation, returns its FD
				throw std::runtime_error("Error while creating the socket");
		// Used to reuse the port (here 8080) and thus avoid the "address already in use" error
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
		this->_address.sin_addr.s_addr = htonl(INADDR_ANY);
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

// Operators
Server & Server::operator=(const Server &assign)
{
	if (this != &assign) {
		_configFile = assign._configFile;
		_socketFd = assign._socketFd;
		_port = assign._port;
		_host = assign._host;
		_root = assign._root;
		_index = assign._index;
		_errorPage = assign._errorPage;
		_cgi = assign._cgi;
		_upload = assign._upload;
		_clientMaxBodySize = assign._clientMaxBodySize;
		_allowMethods = assign._allowMethods;
		_connexions = assign._connexions;
		_address = assign._address;
				_locations = assign._locations;
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


/* TO FIX?
2. Incorrect _connexions.pop_back() Use

You are calling pop_back() regardless of which client FD is being closed. This will remove the wrong connection in most cases.

Fix: Use removeConnexion(fd); instead:

close(events[i].data.fd);
epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
removeConnexion(events[i].data.fd); // FIXED


*/