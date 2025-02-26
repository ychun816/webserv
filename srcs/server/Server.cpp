#include "Server.hpp"

// Constructors
Server::Server()
{
	std::cout << "\e[0;33mDefault Constructor called of Server\e[0m" << std::endl;
}

Server::Server(const Server &copy)
{
	(void) copy;
	std::cout << "\e[0;33mCopy Constructor called of Server\e[0m" << std::endl;
}


// Destructor
Server::~Server()
{
	std::cout << "\e[0;31mDestructor called of Server\e[0m" << std::endl;
}


// Operators
Server & Server::operator=(const Server &assign)
{
	(void) assign;
	return *this;
}

