#include "../includes/webserv.hpp"
#include "../includes/server/Server.hpp"

int main(int argc, char const *argv[])
{
    (void)argv;
    // std::string
    try
    { 
        if (argc == 2)
        {
            Server  server;
            std::string config = argv[1];
            server.init(config);
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
