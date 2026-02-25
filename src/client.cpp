#include "Client.hpp"

/* ================= CONSTRUCTORS ================= */

Client::Client() : _fd(-1) {}

Client::Client(int fd)
    : _fd(fd) {}

Client::~Client() {}

/* ================= GETTERS ================= */

int Client::getFd() const
{
    return _fd;
}

std::string &Client::getBuffer()
{
    return _buffer;
}