#include "../includes/webserv.hpp"
#include "../includes/server/Server.hpp"
#include "../includes/parsing/Config.hpp"
#include "../includes/utils/Utils.hpp"
// error codes : 504  / 501
// siege -b -c 255 -t1M http://127.0.0.1:8080/
// 8081 
// add debug string for fille too large
// verifie ti;eout
// Verifie nginx documentation for multiple ports
int main(int argc, char const *argv[])
{
	(void)argv;
	// std::string
	setupSignalHandler();
	try
	{
		if (argc <= 2)
		{
			Config* config = Config::getInstance(argc == 1 ? "config/baseConfig.txt" : argv[1]);
			config->runServers();
		}
		else
			std::cerr << "Usage: ./webserv  [config file]" << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}
