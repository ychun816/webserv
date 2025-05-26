#ifndef EPOLL_MANAGER_HPP
#define EPOLL_MANAGER_HPP

#include <map>
#include <vector>
#include <sys/epoll.h>

#define MAX_EVENTS_EPOLL 64

class Server;
class Request;

class EpollManager {
private:
    // Instance unique
    static EpollManager* _instance;
    
    // Descripteur epoll
    int _epoll_fd;
    
    // Cartographie des clients aux serveurs
    std::map<int, int> _client_to_server_index;
    
    // Constructeur privé (pattern Singleton)
    EpollManager();
    
    // En C++98, déclarer le constructeur de copie et l'opérateur = en privé
    // au lieu de les marquer = delete
    EpollManager(const EpollManager&);
    EpollManager& operator=(const EpollManager&);
    
public:
    ~EpollManager();
    
    // Obtenir l'instance unique
    static EpollManager* getInstance();
    
    // Initialisation
    bool initialize();
    
    // Ajouter un socket serveur
    bool addServerSocket(int socket_fd, int server_index);
    
    // Ajouter un socket client
    bool addClientSocket(int client_fd, int server_index);
    
    // Supprimer un socket
    bool removeSocket(int fd);
    
    // Boucle principale de traitement des événements
    void processEvents(std::vector<Server>& servers);
    
    // Accesseur
    int getEpollFd() const;
};

#endif