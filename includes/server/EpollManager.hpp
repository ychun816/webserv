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
    static EpollManager* _instance;
    
    int _epoll_fd;
    
    std::map<int, int> _client_to_server_index;
    
    std::map<int, time_t> _connection_times;
    int _timeout;  
    
    EpollManager();
    

    EpollManager(const EpollManager&);
    EpollManager& operator=(const EpollManager&);
    
public:
    ~EpollManager();
    
    static EpollManager* getInstance();
    
    bool initialize();
    
    bool addServerSocket(int socket_fd, int server_index);
    
    bool addClientSocket(int client_fd, int server_index);
    
    bool removeSocket(int fd);
    
    void processEvents(std::vector<Server>& servers);
    
    int getEpollFd() const;
    
    void checkTimeouts(std::vector<Server>& servers);
    void setTimeout(int timeout) { _timeout = timeout; }
};

#endif