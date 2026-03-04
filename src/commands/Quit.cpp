#include "Server.hpp"

void Server::cmdQuit(int fd, const Message &msg)
{
	std::string reason = msg.params.empty() ? "Leaving" : msg.params[0];

	sendTo(fd, "ERROR :Closing Link: " + _clients[fd].getPrefix() + " (Quit: " + reason + ")");
	disconnectClient(fd, reason);
}
