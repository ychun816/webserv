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
    std::string	_configFile;
    std::vector<Server> _servers;
    void initParsing(std::ifstream& file);
    void parseServer(std::vector<std::string>& lines);
    Server fillServer(std::vector<std::string>& lines);
public:
    Config(const std::string& filename);
    ~Config();

    // Getters
    const std::vector<Server>& getServers() const { return _servers; }
    Server& getServer(size_t index) { return _servers.at(index); }
    size_t getServerCount() const { return _servers.size(); }


    // Méthodes utiles
    void addServer(const Server& server) { _servers.push_back(server); }
    void removeServer(size_t index) { _servers.erase(_servers.begin() + index); }
    bool hasServer(size_t index) const { return index < _servers.size(); }
    void addLocation(const std::string& location, Server& server, std::vector<std::string>& lines, std::vector<std::string>::iterator& it);

		class ConfigException : public std::exception
		{
			public:
				ConfigException(const std::string &message) : _message(message) {}
				virtual ~ConfigException() throw() {} 
				virtual const char *what() const throw() { return _message.c_str(); }
			private:
				std::string _message;
		};
};