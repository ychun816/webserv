#pragma once
#include "../parsing/Locations.hpp"

// #include "../../includes/methods/Get.hpp" //added to exec methods
// #include "../../includes/methods/Post.hpp" //added to exec methods
// #include "../../includes/methods/Delete.hpp" //added to exec methods
#include "../../includes/methods/AMethods.hpp" //added to exec methods

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <deque>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <list>
#include "Request.hpp"
#define PORT 8080
#define MAX_EVENTS 1000

class Server
{
        public:
                // Constructors
                Server();
                Server(const Server& other); // Constructeur de copie
                // Destructor
                ~Server();

                // Operators
                Server & operator=(const Server &assign);

                // Methods
                void    createSocket();
                void    configSocket();
                void    runServer();
                void    setNonBlocking();
                void    pushLocation(const Location& location);
                void    handleNewConnection();
                void    setEpollFd(int epoll_fd);
                void    addConnexion(int fd);
                void    removeConnexion(int fd);
                // Getters
                const std::string& getConfigFile() const { return _configFile; }
                const std::deque<int>& getConnexions() const { return _connexions; }
                int getSocketFd() const { return _socketFd; }
                int getPort() const { return _port; }
                const std::string& getHost() const { return _host; }
                const std::string& getRoot() const { return _root; }
                const std::string& getIndex() const { return _index; }
                const std::string& getErrorPage() const { return _errorPage; }
                const std::string& getCgi() const { return _cgi; }
                const std::string& getUpload() const { return _upload; }
                const std::string& getClientMaxBodySize() const { return _clientMaxBodySize; }
                const std::list<std::string>& getAllowMethods() const { return _allowMethods; }
                const std::vector<std::string>& getErrorPages() const { return _errorPages; }
                const std::list<Location>& getLocations() const { return _locations; }
                // Setters
                void setConfigFile(const std::string& configFile) { _configFile = configFile; }
                void setConnexions(const std::deque<int>& connexions) { _connexions = connexions; }
                void setSocketFd(int socketFd) { _socketFd = socketFd; }
                void setPort(int port) { _port = port; }
                void setHost(const std::string& host) { _host = host; }
                void setRoot(const std::string& root) { _root = root; }
                void setIndex(const std::string& index) { _index = index; }
                void setErrorPage(const std::string& errorPage) { _errorPage = errorPage; }
                void setCgi(const std::string& cgi) { _cgi = cgi; }
                void setUpload(const std::string& upload) { _upload = upload; }
                void setClientMaxBodySize(const std::string& clientMaxBodySize) { _clientMaxBodySize = clientMaxBodySize; }
                void setAllowMethods(const std::list<std::string>& allowMethods) { _allowMethods = allowMethods; }
                void setLocations(const std::list<Location>& locations) { _locations = locations; }
  
                //added to exec methods
	              void executeMethods(Request& request, Response& response);//change to server class?
  
        private:
                int                                     _epoll_fd;
                std::string             _configFile;
                std::deque<int>         _connexions;
                int                                     _socketFd;
                int                                     _port;
                std::string                     _host;
                std::string                     _root;
                std::string                     _index;
                std::string                     _errorPage;
                std::string                     _cgi;
                std::string                     _upload;
                std::string                     _clientMaxBodySize;
                std::list<std::string>  _allowMethods;
                struct sockaddr_in      _address;
                std::vector<std::string>        _errorPages;
                std::list<Location>     _locations;

                class configError : public std::exception
                {
                        public:
                                configError(const std::string &message) : _message(message) {}
                                virtual ~configError() throw() {}
                                virtual const char *what() const throw() { return _message.c_str(); }
                        private:
                                std::string _message;
                };

};

// Surcharge de l'opérateur << pour afficher les informations du serveur
inline std::ostream& operator<<(std::ostream& os, const Server& server) {
        os << "Server Configuration:" << std::endl;
        os << "  Port: " << server.getPort() << std::endl;
        os << "  Host: " << server.getHost() << std::endl;
        os << "  Root: " << server.getRoot() << std::endl;
        os << "  Index: " << server.getIndex() << std::endl;
        os << "  Error Page: " << server.getErrorPage() << std::endl;
        os << "  CGI: " << server.getCgi() << std::endl;
        os << "  Upload: " << server.getUpload() << std::endl;
        os << "  Client Max Body Size: " << server.getClientMaxBodySize() << std::endl;
        os << "  Allow Methods: ";
        for (std::list<std::string>::const_iterator it = server.getAllowMethods().begin(); it != server.getAllowMethods().end(); ++it) {
                os << *it << " ";
        }
        std::cout << std::endl;
        std::cout << "  Locations: " << server.getLocations().size() << std::endl;
        os << std::endl;
        return os;
}






// #endif