#include "Server.hpp"

#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_CLIENTS 128

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

	std::cout << "Server listening on port " << _port << std::endl;
}

/* ================= MAIN LOOP ================= */

void Server::run()
{
	while (true)
	{
		if (poll(&_fds[0], _fds.size(), -1) < 0)
			throw std::runtime_error("poll() failed");

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
	std::cout << "Client connected: fd " << clientFd << std::endl;
}

void Server::disconnectClient(int fd)
{
	std::cout << "Client disconnected: fd " << fd << std::endl;

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

	for (std::map<std::string, Channel>::iterator it = _channels.begin();
		 it != _channels.end(); )
	{
		it->second.removeMember(fd);
		if (it->second.empty())
			_channels.erase(it++);
		else
			++it;
	}
}

/* ================= DATA RECEPTION & BUFFERING ================= */

void Server::receiveData(int fd)
{
	char buf[512];
	int  bytes = recv(fd, buf, sizeof(buf) - 1, 0);

	if (bytes <= 0)
	{
		disconnectClient(fd);
		return;
	}
	buf[bytes] = '\0';
	_clients[fd].getBuffer().append(buf);

	// Extract all complete lines before processing to avoid dangling refs
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

	std::cout << "fd " << fd << " >> " << line << std::endl;

	if (msg.command == "CAP")
		return;
	if (msg.command == "PASS")
		return cmdPass(fd, msg);
	if (msg.command == "NICK")
		return cmdNick(fd, msg);
	if (msg.command == "USER")
		return cmdUser(fd, msg);

	if (!_clients[fd].isRegistered())
		return sendNumeric(fd, "451", ":You have not registered");

	// Future commands (JOIN, PRIVMSG, KICK, etc.) dispatched here
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
	sendNumeric(fd, "001", ":Welcome to the IRC Network " + client.getPrefix());
	sendNumeric(fd, "002", ":Your host is " SERVER_NAME ", running version 1.0");
	sendNumeric(fd, "003", ":This server was created today");
	sendNumeric(fd, "004", SERVER_NAME " 1.0 o o");
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
