#include "Server.hpp"

void Server::cmdNotice(int fd, const Message &msg)
{
	if (msg.params.size() < 2)
		return;

	std::string target = msg.params[0];
	std::string text = msg.params[1];
	std::string prefix = ":" + _clients[fd].getPrefix();

	if (target[0] == '#')
	{
		if (_channels.find(target) == _channels.end())
			return;
		Channel &chan = _channels[target];
		if (!chan.hasMember(fd))
			return;
		broadcastToChannel(target, prefix + " NOTICE " + target + " :" + text, fd);
	}
	else
	{
		Client *targetClient = findClientByNick(target);
		if (!targetClient)
			return;
		sendTo(targetClient->getFd(), prefix + " NOTICE " + target + " :" + text);
	}
}
