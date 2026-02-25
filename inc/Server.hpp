#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Client.hpp"

class Server
{
private:
    int _port;
    std::string _password;
    int _serverSocket;
    std::vector<pollfd> _fds;
    std::map<int, Client> _clients;

public:
    Server(int port, const std::string &password);
    ~Server();

    void run();
    void acceptClient();
    void receiveData(int fd);
};

#endif