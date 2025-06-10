#include "../includes/webserv.hpp"
#include "../includes/server/Server.hpp"
#include "../includes/parsing/Config.hpp"
#include "../includes/utils/Utils.hpp"
// html for upload, probleme ports
// error codes : 504  / 501
int main(int argc, char const *argv[])
{
	(void)argv;
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
