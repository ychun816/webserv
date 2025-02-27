#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

class Server
{
	public:
		// Constructors
		Server();
		Server(const std::string & configFile);

		// Destructor
		~Server();
		
		// Operators
		Server & operator=(const Server &assign);

		// Methods
		void	init(std::string &configFile);
		void	createSocket();
		void	configSocket();
	private:
		std::string 		_configFile;
		int					_socketFd;
		struct sockaddr_in 	address;
};
