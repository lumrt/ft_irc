#include "Server.hpp"
#include <cstdlib>
#include <stdexcept>

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(av[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: invalid port number" << std::endl;
        return 1;
    }

    try
    {
        Server server(port, std::string(av[2]));
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}