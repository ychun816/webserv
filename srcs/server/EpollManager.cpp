#include "../../includes/server/EpollManager.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/utils/Utils.hpp"
#include <unistd.h>
#include <iostream>

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

                    char buffer[1024] = {0};
                    int bytes_read = read(current_fd, buffer, sizeof(buffer));

                    if (bytes_read > 0) {
                        std::string request(buffer, bytes_read);
                        std::cout << "Traitement requête client sur serveur " << server_index << std::endl;

                        // Traitement de la requête
                        Request req(request, servers[server_index]);
                        req.handleResponse();
						std::cout << RED << "=========SEND RESPONSE EPOLL==========" << RESET << std::endl;
						std::cout << RED << req.getResponse() << RESET << std::endl;
                        send(current_fd, req.getResponse().c_str(), req.getResponse().size(), 0);

                        // Vérifier keep-alive (si implémenté)
                        // Si pas de keep-alive, fermer la connexion
                    }
                    else {
                        // Client déconnecté
                        std::cout << "Client déconnecté (fd:" << current_fd << ")" << std::endl;
                        close(current_fd);
                        removeSocket(current_fd);
                        servers[server_index].removeConnexion(current_fd);
                    }
                }
            }
        }
    }
}

// Accesseur pour le descripteur epoll
int EpollManager::getEpollFd() const {
    return _epoll_fd;
}