/* FINAL TO DO:
1. cgi-bin root : ./ => unable to go back to the main root 



*/


#include "../includes/webserv.hpp"
#include "../includes/server/Server.hpp"
#include "../includes/parsing/Config.hpp"
#include "../includes/utils/Utils.hpp"

// modifier list locqtions
int main(int argc, char const *argv[])
{
	(void)argv;
	// std::string
	
	setupSignalHandler();

	try
	{
		if (argc <= 2)
		{
			const std::string configFile = (argc == 1 ? "config/baseConfig.txt" : argv[1]);
			Config* config = Config::getInstance(configFile);
			config->runServers();

		}
		else
			std::cerr << "Usage: ./webserv  [config file]" << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	cleanupResources();
	return 0;
}
