#include "Server.hpp"

void Server::cmdPing(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "409", ":No origin specified");
	sendTo(fd, ":" SERVER_NAME " PONG " SERVER_NAME " :" + msg.params[0]);
}
