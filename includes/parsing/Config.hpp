#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "../../includes/webserv.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/utils/Utils.hpp"
#include "../../includes/parsing/Locations.hpp"

class Config
{
private:
    static Config* _instance;
    std::string	_configFile;
    std::vector<Server> _servers;
    void initParsing(std::ifstream& file);
    void parseServer(std::vector<std::string>& lines);
    Server fillServer(std::vector<std::string>& lines);
    void findParameters(std::vector<std::string>::iterator& it, Server& server, std::vector<std::string>& lines);
    
    // Private constructor for Singleton pattern
    explicit Config(const std::string& filename);
    
public:
    
    ~Config();
    
    // Static method to get the singleton instance
    static Config* getInstance(const std::string& filename = "");
    
    // Method to start servers
    void runServers();

    // Getters
    const std::vector<Server>& getServers() const { return _servers; }
    Server& getServer(size_t index) { return _servers.at(index); }
    size_t getServerCount() const { return _servers.size(); }

    // Utility methods
    void addServer(const Server& server) { _servers.push_back(server); }
    void removeServer(size_t index) { _servers.erase(_servers.begin() + index); }
    bool hasServer(size_t index) const { return index < _servers.size(); }
    
    Location parseLocation(const std::string& location, std::vector<std::string>& lines, std::vector<std::string>::iterator& it);

    bool validateServerConfig(const Server& newServer) const;
    Server* findServerByLocation(const std::string& path, int port);

    class ConfigException : public std::exception
    {
        public:
            explicit ConfigException(const std::string& message) : _message(message) {}
            virtual ~ConfigException() throw() {}
            virtual const char* what() const throw() { return _message.c_str(); }
        private:
            std::string _message;
    };
};