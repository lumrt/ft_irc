#include "Server.hpp"

#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_CLIENTS 128

#define RST  "\033[0m"
#define BLD  "\033[1m"
#define RED  "\033[1;31m"
#define GRN  "\033[1;32m"
#define YEL  "\033[1;33m"
#define BLU  "\033[1;34m"
#define MAG  "\033[1;35m"
#define CYN  "\033[1;36m"
#define GRY  "\033[0;90m"

volatile sig_atomic_t Server::_running = 1;

void Server::signalHandler(int sig)
{
	(void)sig;
	_running = 0;
}

/* ================= CONSTRUCTORS ================= */

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverSocket(-1)
{
	initSocket();
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
		close(it->first);
	if (_serverSocket != -1)
		close(_serverSocket);
}

/* ================= INIT ================= */

void Server::initSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("socket() failed");

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");

	fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(_port);

	if (bind(_serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");
	if (listen(_serverSocket, MAX_CLIENTS) < 0)
		throw std::runtime_error("listen() failed");

	pollfd pfd;
	pfd.fd      = _serverSocket;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_fds.push_back(pfd);

	std::cout << GRN << "Server listening on port " << _port << RST << std::endl;
}

/* ================= MAIN LOOP ================= */

void Server::run()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);

	while (_running)
	{
		if (poll(&_fds[0], _fds.size(), -1) < 0)
		{
			if (errno == EINTR)
				continue;
			if (!_running)
				break;
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = 0; i < _fds.size(); ++i)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _serverSocket)
					acceptClient();
				else
					receiveData(_fds[i].fd);
			}
		}
	}
	std::cout << "\n" << GRN << "Server shutting down" << RST << std::endl;
}

/* ================= CONNECTION ================= */

void Server::acceptClient()
{
	struct sockaddr_in clientAddr;
	socklen_t          clientLen = sizeof(clientAddr);
	int clientFd = accept(_serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
	if (clientFd < 0)
		return;

	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	pollfd pfd;
	pfd.fd      = clientFd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_fds.push_back(pfd);

	_clients.insert(std::make_pair(clientFd, Client(clientFd)));
	std::cout << BLU << "[+] " << RST << "Client connected "
			  << GRY << "(fd " << clientFd << ", "
			  << inet_ntoa(clientAddr.sin_addr) << ":"
			  << ntohs(clientAddr.sin_port) << ")" << RST << std::endl;
}

void Server::disconnectClient(int fd, const std::string &reason)
{
	std::string nick = _clients[fd].getNickname();
	std::string prefix = _clients[fd].getPrefix();

	std::cout << RED << "[-] " << RST;
	if (!nick.empty())
		std::cout << BLD << nick << RST;
	else
		std::cout << GRY << "fd " << fd << RST;
	std::cout << " quit " << GRY << "(" << reason << ")" << RST << std::endl;

	std::string quitMsg = ":" + prefix + " QUIT :" + reason;
	for (std::map<std::string, Channel>::iterator it = _channels.begin();
		 it != _channels.end(); )
	{
		if (it->second.hasMember(fd))
		{
			const std::set<int> &members = it->second.getMembers();
			for (std::set<int>::const_iterator m = members.begin();
				 m != members.end(); ++m)
			{
				if (*m != fd)
					sendTo(*m, quitMsg);
			}
			it->second.removeMember(fd);
		}
		if (it->second.empty())
			_channels.erase(it++);
		else
			++it;
	}

	close(fd);
	_clients.erase(fd);

	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_fds.erase(it);
			break;
		}
	}
}

/* ================= DATA RECEPTION & BUFFERING ================= */

void Server::receiveData(int fd)
{
	char buf[512];
	int  bytes = recv(fd, buf, sizeof(buf) - 1, 0);

	if (bytes <= 0)
	{
		disconnectClient(fd, "Connection lost");
		return;
	}
	buf[bytes] = '\0';
	_clients[fd].getBuffer().append(buf);

	std::string             &buffer = _clients[fd].getBuffer();
	std::string::size_type   pos;
	std::vector<std::string> lines;

	while ((pos = buffer.find('\n')) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty())
			lines.push_back(line);
	}

	for (size_t i = 0; i < lines.size(); ++i)
	{
		if (_clients.find(fd) == _clients.end())
			return;
		handleLine(fd, lines[i]);
	}
}

/* ================= COMMAND DISPATCH ================= */

void Server::handleLine(int fd, const std::string &line)
{
	Message msg = parseMessage(line);
	if (msg.command.empty())
		return;

	std::string nick = _clients[fd].getNickname();
	std::string params;
	for (size_t p = 0; p < msg.params.size(); ++p)
	{
		if (p > 0)
			params += " ";
		params += msg.params[p];
	}
	std::cout << CYN << ">> " << RST;
	if (!nick.empty())
		std::cout << BLD << nick << RST;
	else
		std::cout << GRY << "fd " << fd << RST;
	std::cout << " " << GRY << msg.command << RST;
	if (!params.empty())
		std::cout << " " << params;
	std::cout << std::endl;

	if (msg.command == "CAP")
		return;
	if (msg.command == "QUIT")
		return cmdQuit(fd, msg);
	if (msg.command == "PASS")
		return cmdPass(fd, msg);
	if (msg.command == "NICK")
		return cmdNick(fd, msg);
	if (msg.command == "USER")
		return cmdUser(fd, msg);

	if (!_clients[fd].isRegistered())
		return sendNumeric(fd, "451", ":You have not registered");

	if (msg.command == "PING")
		return cmdPing(fd, msg);
	if (msg.command == "PONG")
		return;
	if (msg.command == "JOIN")
		return cmdJoin(fd, msg);
	if (msg.command == "PART")
		return cmdPart(fd, msg);
	if (msg.command == "PRIVMSG")
		return cmdPrivmsg(fd, msg);
	if (msg.command == "NOTICE")
		return cmdNotice(fd, msg);
	if (msg.command == "KICK")
		return cmdKick(fd, msg);
	if (msg.command == "INVITE")
		return cmdInvite(fd, msg);
	if (msg.command == "TOPIC")
		return cmdTopic(fd, msg);
	if (msg.command == "MODE")
		return cmdMode(fd, msg);
	if (msg.command == "WHO")
		return cmdWho(fd, msg);
}

/* ================= SENDING ================= */

void Server::sendTo(int fd, const std::string &message)
{
	std::string full = message + "\r\n";
	send(fd, full.c_str(), full.size(), 0);
}

void Server::sendNumeric(int fd, const std::string &numeric, const std::string &text)
{
	std::string target = _clients[fd].getNickname();
	if (target.empty())
		target = "*";
	sendTo(fd, std::string(":") + SERVER_NAME + " " + numeric + " " + target + " " + text);
}

void Server::broadcastToChannel(const std::string &channel, const std::string &message, int excludeFd)
{
	if (_channels.find(channel) == _channels.end())
		return;
	const std::set<int> &members = _channels[channel].getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it != excludeFd)
			sendTo(*it, message);
	}
}

void Server::sendChannelInfo(int fd, Channel &chan)
{
	if (!chan.getTopic().empty())
		sendNumeric(fd, "332", chan.getName() + " :" + chan.getTopic());

	std::string names;
	const std::set<int> &members = chan.getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!names.empty()) names += " ";
		if (chan.isOperator(*it)) names += "@";
		names += _clients[*it].getNickname();
	}
	sendNumeric(fd, "353", "= " + chan.getName() + " :" + names);
	sendNumeric(fd, "366", chan.getName() + " :End of /NAMES list");
}

/* ================= REGISTRATION ================= */

void Server::tryRegister(int fd)
{
	Client &client = _clients[fd];

	if (client.isRegistered())
		return;
	if (!client.isPassAccepted())
		return;
	if (client.getNickname().empty())
		return;
	if (client.getUsername().empty())
		return;

	client.setRegistered(true);
	std::cout << GRN << "[*] " << RST << BLD << client.getNickname() << RST
			  << " registered " << GRY << "(" << client.getUsername()
			  << ", \"" << client.getRealname() << "\")" << RST << std::endl;
	sendNumeric(fd, "001", ":Welcome to the IRC Network " + client.getPrefix());
	sendNumeric(fd, "002", ":Your host is " SERVER_NAME ", running version 1.0");
	sendNumeric(fd, "003", ":This server was created today");
	sendNumeric(fd, "004", SERVER_NAME " 1.0 itkol itkol");
}

/* ================= LOOKUP ================= */

Client *Server::findClientByNick(const std::string &nickname)
{
	for (std::map<int, Client>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return &it->second;
	}
	return NULL;
}
