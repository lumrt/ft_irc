#include "Client.hpp"

/* ================= CONSTRUCTORS ================= */

Client::Client()
	: _fd(-1), _passAccepted(false), _registered(false) {}

Client::Client(int fd)
	: _fd(fd), _passAccepted(false), _registered(false) {}

Client::~Client() {}

/* ================= GETTERS ================= */

int Client::getFd() const
{
	return _fd;
}

const std::string &Client::getNickname() const
{
	return _nickname;
}

const std::string &Client::getUsername() const
{
	return _username;
}

const std::string &Client::getRealname() const
{
	return _realname;
}

std::string &Client::getBuffer()
{
	return _buffer;
}

bool Client::isPassAccepted() const
{
	return _passAccepted;
}

bool Client::isRegistered() const
{
	return _registered;
}

/* ================= SETTERS ================= */

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

void Client::setPassAccepted(bool accepted)
{
	_passAccepted = accepted;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}

/* ================= UTILS ================= */

std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@localhost";
}
