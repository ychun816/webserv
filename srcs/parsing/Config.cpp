#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <sys/wait.h> // For waitpid
#include <sys/select.h> // For select
#include <sys/epoll.h> // For epoll
#include "../../includes/server/EpollManager.hpp"

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

Config::Config(const std::string& filename) : _configFile(filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + filename);
    timestamp("Parsing configuration file: " + filename, YELLOW);
    initParsing(file);
}

Config::~Config()
{
}
Location Config::parseLocation(const std::string& location, std::vector<std::string>& lines, std::vector<std::string>::iterator& it)
{
    // std::string path = trim(location, " }");
    std::cout << "Path: " << location << std::endl;
    Location NewLocation;
    NewLocation.setPath(location);
    // Move to the end of the location block
    int braceCount = 1; // We start after the opening brace
    it++; // Move to the next line after "location xxx {"

    while (it != lines.end() && braceCount > 0)
    {
        std::cout << "Line: " << *it << std::endl;
        if ((*it).find("{") != std::string::npos)
            braceCount++;
        if ((*it).find("}") != std::string::npos)
            braceCount--;

        if (braceCount == 0)
            break; // We found the corresponding closing brace

        // Process location parameters
        if ((*it).find("methods") != std::string::npos)
        {
            size_t pos = (*it).find("methods") + 7;
            std::string value = (*it).substr(pos);
            std::vector<std::string> methodsVector;
            value = trim(value, " \t;");
            std::stringstream ss(value);
            std::string method;
            while (ss >> method) {
                methodsVector.push_back(method);
            }
            NewLocation.setMethods(methodsVector);
        }
        else if ((*it).find("index") != std::string::npos && (*it).find("autoindex") == std::string::npos)
        {
            size_t pos = (*it).find("index") + 5;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setIndex(value);
        }
        else if ((*it).find("root") != std::string::npos)
        {
            size_t pos = (*it).find("root") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setRoot(value);
        }
        else if ((*it).find("cgi_ext") != std::string::npos)
        {
            size_t pos = (*it).find("cgi_ext") + 7;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setCgiExt(value);
        }
        else if ((*it).find("client_max_body_size") != std::string::npos)
        {
            size_t pos = (*it).find("client_max_body_size") + 20;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setClientMaxBodySize(value);
        }
        else if ((*it).find("autoindex") != std::string::npos)
        {
            size_t pos = (*it).find("autoindex") + 9;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setAutoindex(value);
        }
        else if ((*it).find("upload_path") != std::string::npos)
        {
            size_t pos = (*it).find("upload_path") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            NewLocation.setUploadPath(value);
        }
        else if ((*it).find("return") != std::string::npos)
        {
            size_t pos = (*it).find("return") + 6;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            std::map<size_t, std::string> redirections;
            std::stringstream ss(value);
            std::string code, path;
            while (ss >> code >> path) {
                redirections[atoi(code.c_str())] = path;
            }
            NewLocation.setRedirections(redirections);
        }
        else if ((*it).find("error_page") != std::string::npos)
        {
            size_t pos = (*it).find("error_page") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            std::map<size_t, std::string> errorPages;
            std::stringstream ss(value);
            std::string code, path;
            while (ss >> code >> path) {
                errorPages[atoi(code.c_str())] = path;
            }
            NewLocation.setErrorPage(errorPages);
        }
        it++;
    }
    return (NewLocation);
}

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
 std::cout << "Checking line: [" << *it << "]" << std::endl;
        if ((*it).find("listen") != std::string::npos)
        {
            size_t pos = (*it).find("listen") + 6;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setPort(atoi(value.c_str()));
        }
        else if ((*it).find("server_name") != std::string::npos)
        {
            size_t pos = (*it).find("server_name") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setHost(value);
        }
        else if ((*it).find("methods") != std::string::npos)
        {
            size_t pos = (*it).find("methods") + 7;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            std::list<std::string> methods;
            std::stringstream ss(value);
            std::string method;
            while (ss >> method) {
                methods.push_back(method);
            }
            server.setAllowMethods(methods);
        }
        else if ((*it).find("root") != std::string::npos)
        {
            size_t pos = (*it).find("root") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setRoot(value);
        }
        else if ((*it).find("index") != std::string::npos)
        {
            size_t pos = (*it).find("index") + 5;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setIndex(value);
        }
        else if ((*it).find("client_max_body_size") != std::string::npos)
        {
            size_t pos = (*it).find("client_max_body_size") + 20;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setClientMaxBodySize(value);
        }
        else if ((*it).find("location") != std::string::npos)
        {
            std::cout << "Found location in line: [" << *it << "]" << std::endl;
            size_t pos = (*it).find("location") + 8;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;{");
            std::cout << "Location: " << value << std::endl;

            // parseLocation advances the iterator to the end of the block then we push the location into the server
            Location loc = parseLocation(value, lines, it);
			//printLocation(loc);
            server.pushLocation(loc);
        }
        else if ((*it).find("cgi") != std::string::npos)
        {
            size_t pos = (*it).find("cgi") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setCgi(value);
        }
        else if ((*it).find("error_page") != std::string::npos)
        {
            size_t pos = (*it).find("error_page") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            std::map<size_t, std::string> errorPages;
            std::stringstream ss(value);
            std::string code, path;
            while (ss >> code >> path) {
                errorPages[atoi(code.c_str())] = path;
            }
            server.setErrorPages(errorPages);
        }

}

Server Config::fillServer(std::vector<std::string>& lines)
{
    Server server;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        findParameters(it, server, lines);
    }
    // DEBUG
    for (size_t i = 0; i < lines.size(); i++) {
            std::cout << "Server Line " << i << ": [" << lines[i] << "]" << std::endl;
        }
    return (server);
}
void Config::parseServer(std::vector<std::string>& lines)
{
    if (lines.empty())
        throw ConfigException("No server found");

    // Check if the first line is the beginning of a server block
    if (lines[0] == "server {")
    {
        std::cout << GREEN << "Server found" << RESET << std::endl;
        Server server;
        std::vector<std::string> serverLines;
        int braceCount = 1; // Brace counter to handle nested blocks

        std::vector<std::string>::iterator it = lines.begin() + 1;
        for (; it != lines.end(); ++it)
        {
            // Count braces to ensure finding the end of the current server block
            if ((*it).find("{") != std::string::npos) braceCount++;
            if ((*it).find("}") != std::string::npos) braceCount--;

            // If we find the end of the current server block
            if (braceCount == 0)
            {
                std::cout << GREEN << "Server closed by '}'" << RESET << std::endl;
                server = fillServer(serverLines);
                std::cout << GREEN << "Server filled" << RESET << std::endl;
                std::cout << server << std::endl;

                // Valider la configuration du serveur avant de l'ajouter
                if (validateServerConfig(server)) {
                    addServer(server);
                    std::cout << GREEN << "Server added" << RESET << std::endl;
                } else {
                    throw ConfigException("Invalid server configuration: duplicate location path on same port");
                }

                // Create a new vector with the remaining lines
                std::vector<std::string> remainingLines(it + 1, lines.end());
                // Clean empty lines at the beginning of the vector
                while (!remainingLines.empty() && trim(remainingLines[0], WHITESPACES_WITH_SPACE).empty())
                    remainingLines.erase(remainingLines.begin());

                // If there are remaining lines, continue parsing to find other servers
                if (!remainingLines.empty()) {
                    parseServer(remainingLines);
                }
                return;
            }

            // Add the line to the current server
            std::string trimmedLine = trim(*it, WHITESPACES_WITH_SPACE);
            if (!trimmedLine.empty())
                serverLines.push_back(trimmedLine);
        }

        // If we get here, it means we didn't find the end of the server block
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

    std::cout << "Démarrage de " << _servers.size() << " serveurs..." << std::endl;

    // Obtenir l'instance du singleton
    EpollManager* epollManager = EpollManager::getInstance();

    // Initialiser tous les serveurs
    for (size_t i = 0; i < _servers.size(); i++) {
        std::cout << "Configuration du serveur " << i << " sur port " << _servers[i].getPort() << std::endl;
        _servers[i].createSocket();
        _servers[i].configSocket();

        // Ajouter le socket serveur au epoll centralisé
        epollManager->addServerSocket(_servers[i].getSocketFd(), i);
    }

    // Déléguer le traitement des événements au singleton
    epollManager->processEvents(_servers);
}

// void Config::runServers()
// {
//     if (_servers.empty()) {
//         std::cerr << "No servers to start" << std::endl;
//         return;
//     }

//     std::cout << "Starting " << _servers.size() << " servers..." << std::endl;

//     // Initialize all servers
//     for (size_t i = 0; i < _servers.size(); i++) {
//         std::cout << "Configuring server " << i << " on port " << _servers[i].getPort() << std::endl;
//         _servers[i].createSocket();
//         _servers[i].configSocket();
//     }

//     // Create a single epoll instance for all servers
//     int epoll_fd = epoll_create(1);
//     if (epoll_fd < 0) {
//         std::cerr << "Epoll Error" << std::endl;
//         return;
//     }

//     // Add all server sockets to epoll
//     for (size_t i = 0; i < _servers.size(); i++) {
//         struct epoll_event event;
//         event.events = EPOLLIN;
//         event.data.fd = _servers[i].getSocketFd();
//         _servers[i].setEpollFd(epoll_fd);  // Set epoll_fd for each server
//         if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
//             std::cerr << "Fail to add Socket to epoll" << std::endl;
//             continue;
//         }
//     }

//     // Main loop with epoll
//     struct epoll_event events[MAX_EVENTS];
//     while (true) {
//         int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//         if (event_count == -1) {
//             std::cerr << "epoll_wait error" << std::endl;
//             continue;
//         }

//         for (int i = 0; i < event_count; i++) {
//             // Trouver le serveur correspondant
//             for (size_t j = 0; j < _servers.size(); j++) {
//                 if (events[i].data.fd == _servers[j].getSocketFd()) {
//                     // Nouvelle connexion
//                     _servers[j].handleNewConnection();
//                 } else {
//                     // Connexion client existante
//                     char buffer[1024] = {0};
//                     int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
//                     if (bytes_read > 0) {
//                         std::string request(buffer, bytes_read);
//                         Request req(request, _servers[j]);
//                         req.handleResponse();
//                         send(events[i].data.fd, req.getResponse().c_str(), req.getResponse().size(), 0);

//                     } else {
//                         // Client déconnecté
//                         close(events[i].data.fd);
//                         epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
//                     }
//                 }
//             }
//         }
//     }

//     // Cleanup
//     close(epoll_fd);
// }


void Config::initParsing(std::ifstream& file)
{
    std::string line;
    std::vector<std::string> lines;
    std::cout << GREEN << "/**********PARSING CONFIG FILE**********/" << RESET << std::endl;
    std::cout << CYAN << "----------before parsing----------" << RESET << std::endl;
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
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        std::cout << *it << std::endl;
    }
    std::cout << CYAN << "----------after parsing----------" << RESET << std::endl;
    parseServer(lines);
    std::cout << GREEN << "/**********END OF PARSING**********/" << RESET << std::endl;
    // Don't automatically start servers at the end of parsing
    // runServers();
    // timestamp("Parsing configuration file: " + filename);
}

bool Config::validateServerConfig(const Server& newServer) const
{
    // Vérifier si le port est déjà utilisé par un autre serveur
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getPort() == newServer.getPort()) {
            const std::list<Location>& existingLocations = _servers[i].getLocations();
            const std::list<Location>& newLocations = newServer.getLocations();
            for (std::list<Location>::const_iterator newLoc = newLocations.begin(); newLoc != newLocations.end(); ++newLoc)
            {
                for (std::list<Location>::const_iterator existingLoc = existingLocations.begin(); existingLoc != existingLocations.end(); ++existingLoc)
                {
                    if (newLoc->getPath() == existingLoc->getPath())
                    {
                        std::cerr << "Error: Duplicate location path '" << newLoc->getPath() << "' found on port " << newServer.getPort() << std::endl;
                        return (false);
                    }
                }
            }
        }
    }
    return true;
}

Server* Config::findServerByLocation(const std::string& path, int port)
{
    // Parcourir tous les serveurs qui partagent le même port
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
