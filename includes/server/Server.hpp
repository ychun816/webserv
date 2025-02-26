#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>

class Server
{
	public:
		// Constructors
		Server();
		Server(const Server &copy);
		
		// Destructor
		~Server();
		
		// Operators
		Server & operator=(const Server &assign);
		
	private:
		
};

#endif