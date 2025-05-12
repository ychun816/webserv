#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <sys/wait.h> // For waitpid
#include <sys/select.h> // For select

Config::Config(const std::string& filename) {
    if (filename.empty())
        throw std::runtime_error("Empty config filename");

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
        else if ((*it).find("index") != std::string::npos)
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
        it++;
    }

    return (NewLocation);
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
            server.pushLocation(loc);
        }
        else if ((*it).find("cgi") != std::string::npos)
        {
            size_t pos = (*it).find("cgi") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setCgi(value);
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
                addServer(server);
                std::cout << GREEN << "Server added" << RESET << std::endl;

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
            std::string trimmedLine = trim(*it);
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

void Config::runServers()
{
    if (_servers.empty()) {
        std::cerr << "No servers to start" << std::endl;
        return;
    }

    std::cout << "Starting " << _servers.size() << " servers..." << std::endl;

    // Initialize all servers
    for (size_t i = 0; i < _servers.size(); i++) {
        std::cout << "Configuring server " << i << " on port " << _servers[i].getPort() << std::endl;
        _servers[i].createSocket();
        _servers[i].configSocket();
    }

    // Main loop with select
    fd_set read_fds;
    int max_fd = 0;

    while (true) {
        FD_ZERO(&read_fds);
        
        // Add all server sockets to the set
        for (size_t i = 0; i < _servers.size(); i++) {
            int server_fd = _servers[i].getSocketFd();
            FD_SET(server_fd, &read_fds);
            if (server_fd > max_fd)
                max_fd = server_fd;
        }

        // Wait for activity on any socket
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            std::cerr << "Select error" << std::endl;
            continue;
        }

        // Check each server for new connections
        for (size_t i = 0; i < _servers.size(); i++) {
            if (FD_ISSET(_servers[i].getSocketFd(), &read_fds)) {
                _servers[i].handleNewConnection();
            }
        }
    }
}


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