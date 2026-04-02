#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <csignal>
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

	static volatile sig_atomic_t _running;
	static void signalHandler(int sig);

	void initSocket();
	void acceptClient();
	void receiveData(int fd);
	void handleLine(int fd, const std::string &line);
	void disconnectClient(int fd, const std::string &reason);

	void sendTo(int fd, const std::string &message);
	void sendNumeric(int fd, const std::string &numeric, const std::string &text);
	void broadcastToChannel(const std::string &channel, const std::string &message, int excludeFd);
	void sendChannelInfo(int fd, Channel &chan);

	void cmdPass(int fd, const Message &msg);
	void cmdNick(int fd, const Message &msg);
	void cmdUser(int fd, const Message &msg);
	void cmdQuit(int fd, const Message &msg);
	void cmdJoin(int fd, const Message &msg);
	void cmdPart(int fd, const Message &msg);
	void cmdPrivmsg(int fd, const Message &msg);
	void cmdNotice(int fd, const Message &msg);
	void cmdKick(int fd, const Message &msg);
	void cmdInvite(int fd, const Message &msg);
	void cmdTopic(int fd, const Message &msg);
	void cmdMode(int fd, const Message &msg);
	void cmdPing(int fd, const Message &msg);
	void cmdWho(int fd, const Message &msg);

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
