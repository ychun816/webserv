#include "../../includes/server/EpollManager.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/utils/Utils.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>

// Initialisation de la variable statique
EpollManager* EpollManager::_instance = NULL;

// Constructeur
EpollManager::EpollManager() : _epoll_fd(-1) {
	initialize();
}

// Destructeur
EpollManager::~EpollManager() {
	if (_epoll_fd >= 0) {
		close(_epoll_fd);
	}
}

// Constructeur de copie (privé, non implémenté)
EpollManager::EpollManager(const EpollManager&) {}

// Opérateur d'affectation (privé, non implémenté)
EpollManager& EpollManager::operator=(const EpollManager&) { return *this; }

// Méthode statique pour récupérer l'instance unique
EpollManager* EpollManager::getInstance() {
	if (_instance == NULL) {
		_instance = new EpollManager();
	}
	return _instance;
}

// Initialisation d'epoll
bool EpollManager::initialize() {
	if (_epoll_fd >= 0) {
		return true;  // Déjà initialisé
	}

	_epoll_fd = epoll_create(1);
	if (_epoll_fd < 0) {
		std::cerr << "Erreur lors de la création d'epoll" << std::endl;
		return false;
	}
	return true;
}

// Ajouter un socket serveur à epoll
bool EpollManager::addServerSocket(int socket_fd, int server_index) {
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = socket_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
		std::cerr << "Erreur lors de l'ajout du socket serveur à epoll" << std::endl;
		return false;
	}

	(void)server_index;
	return true;
}

// Ajouter un socket client à epoll
bool EpollManager::addClientSocket(int client_fd, int server_index) {
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		std::cerr << "Erreur lors de l'ajout du socket client à epoll" << std::endl;
		return false;
	}

	// Association cruciale entre le client et son serveur
	_client_to_server_index[client_fd] = server_index;
	return true;
}

// Supprimer un socket d'epoll
bool EpollManager::removeSocket(int fd) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	_client_to_server_index.erase(fd);  // Supprimer l'association
	return true;
}

// Processus principal de traitement des événements
void EpollManager::processEvents(std::vector<Server>& servers) {
	struct epoll_event events[MAX_EVENTS_EPOLL];

	// Add a map to store incomplete requests
	std::map<int, std::string> incomplete_requests;
	std::map<int, size_t> expected_sizes;

	while (true) {
		int event_count = epoll_wait(_epoll_fd, events, MAX_EVENTS_EPOLL, -1);
		if (event_count == -1) {
			std::cerr << "Erreur epoll_wait" << std::endl;
			continue;
		}

		for (int i = 0; i < event_count; i++) {
			int current_fd = events[i].data.fd;
			bool is_server_socket = false;
			int server_index = -1;

			// Vérifier si c'est un socket serveur
			for (size_t j = 0; j < servers.size(); j++) {
				if (current_fd == servers[j].getSocketFd()) {
					is_server_socket = true;
					server_index = j;
					break;
				}
			}

			if (is_server_socket) {
				// Nouvelle connexion - accepter et associer au bon serveur
				struct sockaddr_in client_addr;
				socklen_t client_addrlen = sizeof(client_addr);
				int client_fd = accept(current_fd, (struct sockaddr*)&client_addr, &client_addrlen);

				if (client_fd < 0) {
					std::cerr << "Échec accept" << std::endl;
					continue;
				}

				std::cout << "Nouvelle connexion acceptée sur serveur " << server_index
						  << " (fd:" << client_fd << ")" << std::endl;

				// Ajouter le client au serveur et à epoll
				servers[server_index].addConnexion(client_fd);
				addClientSocket(client_fd, server_index);
			}
			else {
				// Connexion client existante - utiliser la map pour trouver le bon serveur
				std::map<int, int>::iterator it = _client_to_server_index.find(current_fd);
				if (it != _client_to_server_index.end()) {
					int server_index = it->second;

					// In your event processing loop where you read from sockets
					char buffer[8192]; // Decent buffer size
					int bytes_read = read(current_fd, buffer, sizeof(buffer));

					if (bytes_read <= 0) {
						// Client disconnected
						std::cout << "Client déconnecté (fd:" << current_fd << ")" << std::endl;
						close(current_fd);
						removeSocket(current_fd);
						servers[server_index].removeConnexion(current_fd);

						// Clean up any incomplete requests
						if (incomplete_requests.find(current_fd) != incomplete_requests.end()) {
							incomplete_requests.erase(current_fd);
							expected_sizes.erase(current_fd);
						}
						continue;
					}

					// Add the new data to any existing incomplete request
					if (incomplete_requests.find(current_fd) == incomplete_requests.end()) {
						incomplete_requests[current_fd] = std::string(buffer, bytes_read);
					} else {
						incomplete_requests[current_fd].append(buffer, bytes_read);
					}

					std::string& current_request = incomplete_requests[current_fd];

					// On first read, try to determine Content-Length to know when request is complete
					if (expected_sizes.find(current_fd) == expected_sizes.end()) {
						// New request - look for Content-Length header
						size_t header_pos = current_request.find("Content-Length: ");

						if (header_pos != std::string::npos) {
							size_t value_start = header_pos + 16; // Length of "Content-Length: "
							size_t value_end = current_request.find("\r\n", value_start);

							if (value_end != std::string::npos) {
								std::string length_str = current_request.substr(value_start, value_end - value_start);
								try {
									std::istringstream iss(length_str);
									size_t content_length = 0;
									iss >> content_length;
									
									// Find where headers end and body begins
									size_t body_start = current_request.find("\r\n\r\n");
									if (body_start != std::string::npos) {
										body_start += 4; // Skip over the \r\n\r\n

										// Total expected size is headers + body
										size_t expected_total = body_start + content_length;
										expected_sizes[current_fd] = expected_total;
									}
								} catch(...) {
									// If we can't parse the Content-Length, assume it's complete
									expected_sizes[current_fd] = current_request.length();
								}
							}
						} else {
							// No Content-Length found, assume it's complete
							expected_sizes[current_fd] = current_request.length();
						}
					}

					// Check if we've received the complete request
					if (current_request.length() >= expected_sizes[current_fd]) {
						// We have a complete request
						std::cout << "Traitement requête client sur serveur " << server_index << std::endl;

						// Create a request object
						Request req(current_request, servers[server_index]);

						if (req.isBodySizeValid()) {
							// Process request and send response
							req.handleResponse();
							// std::cout << "=========SEND RESPONSE EPOLL==========" << std::endl;
							// std::cout << req.getResponse() << std::endl;

							send(current_fd, req.getResponse().c_str(), req.getResponse().size(), 0);
						} else {
							// Send 413 Request Entity Too Large
							std::string error_response = "HTTP/1.1 413 Request Entity Too Large\r\n"
														  "Content-Type: text/html\r\n"
														  "Content-Length: 67\r\n\r\n"
														  "<html><body><h1>Error: File too large.</h1></body></html>";
							std::cout << "=========SEND ERROR RESPONSE==========" << std::endl;
							std::cout << error_response << std::endl;
							// Send error response
							send(current_fd, error_response.c_str(), error_response.size(), 0);
						}

						// Clean up after processing
						incomplete_requests.erase(current_fd);
						expected_sizes.erase(current_fd);
					}
					// If we haven't received the full request yet, we'll wait for more data
				}
			}
		}
	}
}

// Accesseur pour le descripteur epoll
int EpollManager::getEpollFd() const {
	return _epoll_fd;
}