#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <string>
#include <sys/wait.h>   // For waitpid
#include <sys/select.h> // For select
#include <sys/epoll.h>  // For epoll
#include "../../includes/server/EpollManager.hpp"
#include <arpa/inet.h>  // For inet_pton

// Initialisation de l'instance statique
Config* Config::_instance = 0;

Config* Config::getInstance(const std::string& filename)
{
    if (_instance == 0)
    {
        if (filename.empty())
            throw ConfigException("Cannot create Config instance without filename");
        _instance = new Config(filename);
    }
    return (_instance);
}

void Config::addServer(const Server& server)
{
    Server newServer = server;
    newServer.setEpollFd(-1);   
    newServer.setSocketFd(-1);  
    _servers.push_back(newServer);
}

Config::Config(const std::string& filename) : _configFile(filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + filename);
    initParsing(file);
}

Config::~Config()
{
}
Location Config::parseLocation(const std::string& location, std::vector<std::string>& lines, std::vector<std::string>::iterator& it)
{
    Location NewLocation;
    NewLocation.setPath(location);
    int braceCount = 1;
    it++;

    while (it != lines.end() && braceCount > 0)
    {
        if ((*it).find("{") != std::string::npos)
            braceCount++;
        if ((*it).find("}") != std::string::npos)
            braceCount--;

        if (braceCount == 0)
            break;

        std::string line = *it;
        std::istringstream iss(line);
        std::string directive;
        iss >> directive;

        if (directive == "methods")
        {
            std::string value;
            std::getline(iss, value);
            value = trim(value, " \t;");
            std::vector<std::string> methodsVector;
            std::stringstream ss(value);
            std::string method;
            while (ss >> method) {
                methodsVector.push_back(method);
            }
            NewLocation.setMethods(methodsVector);
        }
        else if (directive == "index")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setIndex(value);
        }
        else if (directive == "root")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setRoot(value);
        }
        else if (directive == "cgi_ext")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setCgiExt(value);
        }
        else if (directive == "client_max_body_size")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setClientMaxBodySize(value);
        }
        else if (directive == "autoindex")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setAutoindex(value);
        }
        else if (directive == "upload_path")
        {
            std::string value;
            iss >> value;
            value = trim(value, " \t;");
            NewLocation.setUploadPath(value);
        }
        else if (directive == "return")
        {
            int code;
            std::string url;
            iss >> code >> url;
            url = trim(url, " \t;");
            
            if (code != 301 && code != 302 && code != 303 && 
                code != 307 && code != 308) {
                std::ostringstream oss;
                oss << code;
                throw std::runtime_error("Code de redirection invalide: " + oss.str());
            }
            
            NewLocation.setReturn(code, url);
        }
        else if ((*it).find("error_page") != std::string::npos)
        {
            size_t pos = (*it).find("error_page") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            
            // Extraire le code et le chemin
            size_t spacePos = value.find(' ');
            if (spacePos != std::string::npos) {
                std::string code = value.substr(0, spacePos);
                std::string path = value.substr(spacePos + 1);
                path = trim(path, " \t;");
                
                // Nettoyer le chemin
                if (!path.empty() && path[0] == '"') {
                    path = path.substr(1);
                }
                if (!path.empty() && path[path.length()-1] == '"') {
                    path = path.substr(0, path.length()-1);
                }
                
                NewLocation.setErrorPage(std::make_pair(atoi(code.c_str()), path));
            }
        }
        it++;
    }
    return (NewLocation);
}

//DEBUGGER? //////////////////////////////////////////////////////////////////
void	printLocation(Location& loc)
{
	std::cout << std::endl;
	std::cout << "===========PRINT LOCATION===========" << std::endl;
	std::cout << "loc.getPath : " << loc.getPath() << std::endl;
    std::cout << "loc.getRoot : " << loc.getRoot() << std::endl;
    std::cout << "loc.getAutoindex : " << loc.getAutoindex() << std::endl;
    std::cout << "loc.getCgiExt : " << loc.getCgiExt() << std::endl;
    std::cout << "loc.getClientMaxBodySize : " << loc.getClientMaxBodySize() << std::endl;
    std::cout << "loc.getUploadPath : " << loc.getUploadPath() << std::endl;
    std::cout << "loc.getIndex : " << loc.getIndex() << std::endl;
	std::cout << "===========END PRINT LOCATION===========" << std::endl;
	std::cout << std::endl;
}

void Config::findParameters(std::vector<std::string>::iterator& it, Server& server, std::vector<std::string>& lines)
{
    std::string line = *it;
    std::istringstream iss(line);
    std::string directive;
    iss >> directive;

    if (directive == "listen")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setPort(atoi(value.c_str()));
    }
    else if (directive == "server_name")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        // Validation du nom de serveur
        if (value.empty() || value.find_first_of(" \t") != std::string::npos) {
            throw ConfigException("Invalid server_name format");
        }
        server.setServerName(value);
    }
    else if (directive == "host")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        // Validation de l'adresse IP
        struct in_addr addr;
        if (inet_pton(AF_INET, value.c_str(), &addr) != 1) {
            throw ConfigException("Invalid IP address format in host directive");
        }
        server.setHost(value);
    }
    else if (directive == "methods")
    {
        std::string value;
        std::getline(iss, value);
        value = trim(value, " \t;");
        std::list<std::string> methods;
        std::stringstream ss(value);
        std::string method;
        while (ss >> method) {
            methods.push_back(method);
        }
        server.setAllowMethods(methods);
    }
    else if (directive == "root")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setRoot(value);
    }
    else if (directive == "index")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setIndex(value);
    }
    else if (directive == "client_max_body_size")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setClientMaxBodySize(value);
    }
    else if (directive == "location")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;{");

        Location loc = parseLocation(value, lines, it);
        server.pushLocation(loc);
    }
    else if (directive == "autoindex")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setAutoIndex(value);
    }
    else if (directive == "cgi")
    {
        std::string value;
        iss >> value;
        value = trim(value, " \t;");
        server.setCgi(value);
    }
    else if ((*it).find("error_page") != std::string::npos)
    {
            size_t pos = (*it).find("error_page") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            
            // Extraire le code et le chemin
            size_t spacePos = value.find(' ');
            if (spacePos != std::string::npos) {
                std::string code = value.substr(0, spacePos);
                std::string path = value.substr(spacePos + 1);
                path = trim(path, " \t;");
                
                // Nettoyer le chemin
                if (!path.empty() && path[0] == '"') {
                    path = path.substr(1);
                }
                if (!path.empty() && path[path.length()-1] == '"') {
                    path = path.substr(0, path.length()-1);
                }
                
                server.setErrorPages(std::make_pair(atoi(code.c_str()), path));
        }
    }
    else if (directive == "return")
    {
        int code;
        std::string url;
        iss >> code >> url;
        
        url = trim(url, " \t;");
        
        if (code != 301 && code != 302 && code != 303 && 
            code != 307 && code != 308) {
            std::ostringstream oss;
            oss << code;
            throw std::runtime_error("Code de redirection invalide: " + oss.str());
        }
        
        server.setReturn(code, url);
    }
}

Server Config::fillServer(std::vector<std::string>& lines)
{
    Server server;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        findParameters(it, server, lines);
    }

    return (server);
}

void Config::parseServer(std::vector<std::string>& lines)
{
    if (lines.empty())
        throw ConfigException("No server found");

    
    if (lines[0] == "server {")
    {
        Server server;
        std::vector<std::string> serverLines;
        int braceCount = 1; 

        std::vector<std::string>::iterator it = lines.begin() + 1;
        for (; it != lines.end(); ++it)
        {
            if ((*it).find("{") != std::string::npos) braceCount++;
            if ((*it).find("}") != std::string::npos) braceCount--;

            if (braceCount == 0)
            {
                server = fillServer(serverLines);
                
                if (validateServerConfig(server)) {
                    addServer(server);
                } else {
                    throw ConfigException("Invalid server configuration: duplicate location path on same port");
                }

                std::vector<std::string> remainingLines(it + 1, lines.end());

                while (!remainingLines.empty() && trim(remainingLines[0], WHITESPACES_WITH_SPACE).empty())
                    remainingLines.erase(remainingLines.begin());

                if (!remainingLines.empty()) {
                    parseServer(remainingLines);
                }
                return;
            }

            std::string trimmedLine = trim(*it, WHITESPACES_WITH_SPACE);
            if (!trimmedLine.empty())
                serverLines.push_back(trimmedLine);
        }

        if (braceCount > 0)
            throw ConfigException("Unclosed server block");
    }
    else
        throw ConfigException("No server found");
}

void Config::runServers() {
    if (_servers.empty()) {
        std::cerr << "Pas de serveurs à démarrer" << std::endl;
        return;
    }


    EpollManager* epollManager = EpollManager::getInstance();

    for (size_t i = 0; i < _servers.size(); i++) {
        _servers[i].createSocket();
        _servers[i].configSocket();

        epollManager->addServerSocket(_servers[i].getSocketFd(), i);
    }

    epollManager->processEvents(_servers);
}

void Config::initParsing(std::ifstream& file)
{
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line))
    {
        line = trim(line, WHITESPACES);
        if (line.empty() || line[0] == '#')
            continue;
        lines.push_back(line);
    }
    if (!lines.size())
    {
        timestamp("No valid lines found in config file", RED);
        exit(1);
    }
    parseServer(lines);

}

bool Config::validateServerConfig(const Server& newServer) const
{
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getPort() == newServer.getPort()) {
            const std::list<Location>& existingLocations = _servers[i].getLocations();
            const std::list<Location>& newLocations = newServer.getLocations();
            for (std::list<Location>::const_iterator newLoc = newLocations.begin(); newLoc != newLocations.end(); ++newLoc)
            {
                for (std::list<Location>::const_iterator existingLoc = existingLocations.begin(); existingLoc != existingLocations.end(); ++existingLoc)
                {
                    // if (newLoc->getPath() == existingLoc->getPath())
                    // {
                    //     std::cerr << "Error: Duplicate location path '" << newLoc->getPath() << "' found on port " << newServer.getPort() << std::endl;
                    //     return (false);
                    // }
                }
            }
        }
    }
    return true;
}

Server* Config::findServerByLocation(const std::string& path, int port)
{
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getPort() == port)
        {
            const std::list<Location>& locations = _servers[i].getLocations();
            for (std::list<Location>::const_iterator loc = locations.begin(); loc != locations.end(); ++loc)
            {
                if (path.find(loc->getPath()) == 0)
                    return (&_servers[i]);
            }
        }
    }
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getPort() == port)
            return (&_servers[i]);
    }
    return NULL;
}

Server* Config::findServerByHost(const std::string& host, int port) {
    Server* defaultServer = NULL;
    
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
        // Vérifie d'abord si le port correspond
        if (it->getPort() == port) {
            // Si c'est le premier serveur trouvé pour ce port, on le garde comme serveur par défaut
            if (!defaultServer) {
                defaultServer = &(*it);
            }
            
            // Vérifie si le server_name correspond
            if (it->getServerName() == host) {
                return &(*it);
            }
        }
    }
    
    // Si aucun serveur ne correspond exactement, retourne le serveur par défaut
    return defaultServer;
}

void Config::stopServers()
{
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getSocketFd() > 0)
        {
            _servers[i].removeAllConnexions();
            close(_servers[i].getSocketFd());
        }

        int epoll_fd = _servers[i].getEpollFd();
        if (epoll_fd > 0 && _servers[i].getSocketFd() > 0)
        {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _servers[i].getSocketFd(), NULL);
        }
    }
}