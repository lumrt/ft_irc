#include "Server.hpp"

// ERR_NEEDMOREPARAMS (461)
// ERR_ALREADYREGISTRED (462)

void Server::cmdUser(int fd, const Message &msg)
{
	Client &client = _clients[fd];

	if (client.isRegistered())
		return sendNumeric(fd, "462", ":You may not reregister");
	if (msg.params.size() < 4)
		return sendNumeric(fd, "461", "USER :Not enough parameters");

	client.setUsername(msg.params[0]);
	client.setRealname(msg.params[3]);

	tryRegister(fd);
}
