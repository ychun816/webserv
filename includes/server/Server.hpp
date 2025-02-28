#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <deque>
#include <unistd.h>
#include <fcntl.h>

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
		void	runServer();
	private:
		std::string 		_configFile;
		std::deque<int>		_connexions;
		int					_socketFd;
		struct sockaddr_in 	address;
};
