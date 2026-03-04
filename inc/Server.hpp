#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"

#define SERVER_NAME "ircserv"

class Server
{
private:
	int                            _port;
	std::string                    _password;
	int                            _serverSocket;
	std::vector<pollfd>            _fds;
	std::map<int, Client>          _clients;
	std::map<std::string, Channel> _channels;

	void initSocket();
	void acceptClient();
	void receiveData(int fd);
	void handleLine(int fd, const std::string &line);
	void disconnectClient(int fd, const std::string &reason);

	void sendTo(int fd, const std::string &message);
	void sendNumeric(int fd, const std::string &numeric, const std::string &text);

	void cmdPass(int fd, const Message &msg);
	void cmdNick(int fd, const Message &msg);
	void cmdUser(int fd, const Message &msg);
	void cmdQuit(int fd, const Message &msg);

	void    tryRegister(int fd);
	Client *findClientByNick(const std::string &nickname);

	Server(const Server &other);
	Server &operator=(const Server &other);

public:
	Server(int port, const std::string &password);
	~Server();

	void run();
};

#endif
