#pragma once
#include "../parsing/Locations.hpp"
#include "../utils/Utils.hpp"
#include "../methods/AMethods.hpp"



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
#define DEFAULT_TIMEOUT 60 // Timeout par défaut en secondes
#define DEFAULT_KEEP_ALIVE_TIMEOUT 5 // Timeout keep-alive par défaut en secondes


class Server
{
		public:
				// Constructor & Destructor
				Server();
				Server(const Server& other);
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
				bool    errorPageExist(size_t code);
				bool	isServerNameMatch(const std::string& host) const;
				void    removeAllConnexions();
				

				// Getters
				const std::string& getConfigFile() const { return _configFile; }
				const std::deque<int>& getConnexions() const { return _connexions; }
				int   getSocketFd() const { return _socketFd; }
				int   getPort() const { return _port; }
				const std::string& getHost() const { return _host; }
				const std::string& getRoot() const { return _root; }
				const std::string& getIndex() const { return _index; }
				const std::string& getAutoIndex() const { return _autoIndex; }
				const std::string& getErrorPage() const { return _errorPage; }
				const std::string& getCgi() const { return _cgi; }
				const std::string& getUpload() const { return _upload; }
				const std::string& getClientMaxBodySize() const { return _clientMaxBodySize; }
				const std::list<std::string>& getAllowMethods() const { return _allowMethods; }
				const std::map<size_t, std::string>& getErrorPages() const { return _errorPages; }
				const std::list<Location>& getLocations() const { return _locations; }
				const std::pair<size_t, std::string>& getReturn() const { return _return; }
				const std::string& getServerName() const { return _serverName; }
				int getEpollFd() const { return _epoll_fd; }
				int getTimeout() const { return _timeout; }
				int getKeepAliveTimeout() const { return _keepAliveTimeout; }

				// Setters
				void setConfigFile(const std::string& configFile) { _configFile = configFile; }
				void setConnexions(const std::deque<int>& connexions) { _connexions = connexions; }
				void setSocketFd(int socketFd) { _socketFd = socketFd; }
				void setPort(int port) { _port = port; }
				void setHost(const std::string& host) { _host = host; }
				void setRoot(const std::string& root) { _root = root; }
				void setIndex(const std::string& index) { _index = index; }
				void setAutoIndex(const std::string& autoIndex) { _autoIndex = autoIndex; }
				void setErrorPages(const std::pair<size_t, std::string>& errorPage) { _errorPages.insert(errorPage); }
				void setCgi(const std::string& cgi) { _cgi = cgi; }
				void setUpload(const std::string& upload) { _upload = upload; }
				void setClientMaxBodySize(const std::string& clientMaxBodySize) { _clientMaxBodySize = clientMaxBodySize; }
				void setAllowMethods(const std::list<std::string>& allowMethods) { _allowMethods = allowMethods; }
				void setLocations(const std::list<Location>& locations) { _locations = locations; }
				void setReturn(int code, const std::string& url) { _return = std::make_pair(code, url); }
				void setServerName(const std::string& serverName) { _serverName = serverName; }
				void executeMethods(Request& request, Response& response);
				Location* getCurrentLocation(const std::string& path);
				void setTimeout(int timeout) { _timeout = timeout; }
				void setKeepAliveTimeout(int timeout) { _keepAliveTimeout = timeout; }
				void checkTimeouts();

		private:
				int                                     _epoll_fd;
				std::string                             _configFile;
				std::deque<int>                          _connexions;
				int                                     _socketFd;
				int                                     _port;
				std::string                             _host;
				std::string                             _serverName; 
				std::string                             _root;
				std::string                             _index;
				std::string                             _autoIndex;
				std::string                             _errorPage;
				std::string                             _cgi;
				std::string                             _upload;
				std::string                             _clientMaxBodySize;
				std::list<std::string>                  _allowMethods;
				struct sockaddr_in                      _address;
				std::map<size_t, std::string>           _errorPages;
				std::list<Location>                     _locations;
				std::pair<size_t, std::string>          _return;
				int                                     _timeout;           
				int                                     _keepAliveTimeout;  
				std::map<int, time_t>                   _connectionTimes;   
				std::string                             _httpVersion;
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
inline std::ostream& operator<<(std::ostream& os, const Server& server) 
{
		os << "Server Configuration:" << std::endl;
		os << "  Port: " << server.getPort() << std::endl;
		os << "  Host: " << server.getHost() << std::endl;
		os << "  Root: " << server.getRoot() << std::endl;
		os << "  Index: " << server.getIndex() << std::endl;
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
