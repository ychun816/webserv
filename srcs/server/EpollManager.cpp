#include "../../includes/server/EpollManager.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/utils/Utils.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>

EpollManager* EpollManager::_instance = NULL;

EpollManager::EpollManager() : _epoll_fd(-1), _timeout(15) {
	initialize();
}

EpollManager::~EpollManager() {
	if (_epoll_fd >= 0) {
		close(_epoll_fd);
	}
}

EpollManager::EpollManager(const EpollManager&) {}

EpollManager& EpollManager::operator=(const EpollManager&) { return *this; }

EpollManager* EpollManager::getInstance() {
	if (_instance == NULL) {
		_instance = new EpollManager();
	}
	return _instance;
}

bool EpollManager::initialize() {
	if (_epoll_fd >= 0) {
		return true;
	}

	_epoll_fd = epoll_create(1);
	if (_epoll_fd < 0) {
		std::cerr << "Erreur lors de la création d'epoll" << std::endl;
		return false;
	}
	return true;
}

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

bool EpollManager::addClientSocket(int client_fd, int server_index) {
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		std::cerr << "Erreur lors de l'ajout du socket client à epoll" << std::endl;
		return false;
	}

	_client_to_server_index[client_fd] = server_index;
	_connection_times[client_fd] = time(NULL);
	return true;
}

bool EpollManager::removeSocket(int fd) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	_client_to_server_index.erase(fd);
	return true;
}

void EpollManager::processEvents(std::vector<Server>& servers) {
	struct epoll_event events[MAX_EVENTS_EPOLL];
	std::map<int, std::string> incomplete_requests;
	std::map<int, size_t> expected_sizes;

	while (true) {
		int event_count = epoll_wait(_epoll_fd, events, MAX_EVENTS_EPOLL, 1000);

		checkTimeouts(servers);

		if (event_count == -1) {
			if (errno == EINTR) continue;
			std::cerr << "Erreur epoll_wait" << std::endl;
			continue;
		}

		for (int i = 0; i < event_count; i++) {
			int current_fd = events[i].data.fd;
			bool is_server_socket = false;
			int server_index = -1;

			for (size_t j = 0; j < servers.size(); j++) {
				if (current_fd == servers[j].getSocketFd()) {
					is_server_socket = true;
					server_index = j;
					break;
				}
			}

			if (is_server_socket) {
				struct sockaddr_in client_addr;
				socklen_t client_addrlen = sizeof(client_addr);
				int client_fd = accept(current_fd, (struct sockaddr*)&client_addr, &client_addrlen);

				if (client_fd < 0) {
					std::cerr << "Échec accept" << std::endl;
					continue;
				}

				std::cout << "Nouvelle connexion acceptée sur serveur " << server_index
						  << " (fd:" << client_fd << ")" << std::endl;

				servers[server_index].addConnexion(client_fd);
				addClientSocket(client_fd, server_index);
			}
			else {
				std::map<int, int>::iterator it = _client_to_server_index.find(current_fd);
				if (it != _client_to_server_index.end()) {
					int server_index = it->second;

					char buffer[8192];
					int bytes_read = read(current_fd, buffer, sizeof(buffer));

					if (bytes_read <= 0) {
						std::cout << "Client déconnecté (fd:" << current_fd << ")" << std::endl;
						close(current_fd);
						removeSocket(current_fd);
						servers[server_index].removeConnexion(current_fd);

						if (incomplete_requests.find(current_fd) != incomplete_requests.end()) {
							incomplete_requests.erase(current_fd);
							expected_sizes.erase(current_fd);
						}
						continue;
					}

					if (incomplete_requests.find(current_fd) == incomplete_requests.end()) {
						incomplete_requests[current_fd] = std::string(buffer, bytes_read);
					} else {
						incomplete_requests[current_fd].append(buffer, bytes_read);
					}

					std::string& current_request = incomplete_requests[current_fd];

					if (expected_sizes.find(current_fd) == expected_sizes.end()) {
						size_t header_pos = current_request.find("Content-Length: ");

						if (header_pos != std::string::npos) {
							size_t value_start = header_pos + 16;
							size_t value_end = current_request.find("\r\n", value_start);

							if (value_end != std::string::npos) {
								std::string length_str = current_request.substr(value_start, value_end - value_start);
								try {
									std::istringstream iss(length_str);
									size_t content_length = 0;
									iss >> content_length;

									if (content_length > MAX_REQUEST_SIZE) {
										std::string error_response = "HTTP/1.1 413 Request Entity Too Large\r\n"
																			  "Content-Type: text/html\r\n"
																			  "Content-Length: 67\r\n\r\n"
																			  "<html><body><h1>Error: File too large.</h1></body></html>";
										std::cout << "=========SEND ERROR RESPONSE==========" << std::endl;
										std::cout << error_response << std::endl;
										debugString(413);
										send(current_fd, error_response.c_str(), error_response.size(), 0);
										incomplete_requests.erase(current_fd);
										expected_sizes.erase(current_fd);
										continue;
									}

									size_t body_start = current_request.find("\r\n\r\n");
									if (body_start != std::string::npos) {
										body_start += 4;
										size_t expected_total = body_start + content_length;
										expected_sizes[current_fd] = expected_total;
									}
								} catch(...) {
									expected_sizes[current_fd] = current_request.length();
								}
							}
						} else {
							expected_sizes[current_fd] = current_request.length();
						}
					}

					if (current_request.length() >= expected_sizes[current_fd]) {
						std::cout << "Traitement requête client sur serveur " << server_index << std::endl;

						Request req(current_request, servers[server_index]);
						if (req.isBodySizeValid()) {
							req.handleResponse();

							send(current_fd, req.getResponse().c_str(), req.getResponse().size(), 0);

							if (req.getHeader("Connection") != "keep-alive") {
								close(current_fd);
								removeSocket(current_fd);
								servers[server_index].removeConnexion(current_fd);
								std::cout << "Connection closed for client (fd:" << current_fd << ")" << std::endl;
							}
							else {
								// Keep the connection open for further requests
								std::cout << "Connection kept alive for client (fd:" << current_fd << ")" << std::endl;
							}
						} else {
							std::string error_response = "HTTP/1.1 413 Request Entity Too Large\r\n"
														  "Content-Type: text/html\r\n"
														  "Content-Length: 67\r\n\r\n"
														  "<html><body><h1>Error: File too large.</h1></body></html>";
							std::cout << "=========SEND ERROR RESPONSE==========" << std::endl;
							std::cout << error_response << std::endl;
							send(current_fd, error_response.c_str(), error_response.size(), 0);
						}

						incomplete_requests.erase(current_fd);
						expected_sizes.erase(current_fd);
					}
				}
			}
		}
	}
}

void EpollManager::checkTimeouts(std::vector<Server>& servers) {
	time_t current_time = time(NULL);

	std::vector<std::pair<int, int> > to_check;
	for (std::map<int, int>::const_iterator it = _client_to_server_index.begin();
		 it != _client_to_server_index.end(); ++it) {
		to_check.push_back(*it);
	}

	for (std::vector<std::pair<int, int> >::const_iterator it = to_check.begin();
		 it != to_check.end(); ++it) {
		int fd = it->first;
		int server_index = it->second;

		if (_connection_times.find(fd) == _connection_times.end()) {
			continue;
		}

		time_t connection_time = _connection_times[fd];
		time_t elapsed_time = current_time - connection_time;

		if (elapsed_time > _timeout) {
			std::cout << "\033[31mConnection " << fd << " timed out after " << elapsed_time << " seconds\033[0m" << std::endl;
			close(fd);
			removeSocket(fd);
			servers[server_index].removeConnexion(fd);
			_connection_times.erase(fd);
			_client_to_server_index.erase(fd);
		}
	}
}

int EpollManager::getEpollFd() const {
	return _epoll_fd;
}