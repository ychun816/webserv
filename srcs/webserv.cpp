#include "../includes/webserv.hpp"
#include "../includes/server/Server.hpp"
#include "../includes/parsing/Config.hpp"
int main(int argc, char const *argv[])
{
    (void)argv;
    // std::string
    try
    { 
        if (argc <= 2)
        {
            // Server  server;
			const std::string configFile = (argc == 1 ? "config/baseConfig.txt" : argv[1]);
            Config config(configFile);
            // server.init(config);
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
