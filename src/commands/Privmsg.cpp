#include "Server.hpp"

void Server::cmdPrivmsg(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "411", ":No recipient given (PRIVMSG)");
	if (msg.params.size() < 2)
		return sendNumeric(fd, "412", ":No text to send");

	std::string target = msg.params[0];
	std::string text = msg.params[1];
	std::string prefix = ":" + _clients[fd].getPrefix();

	if (target[0] == '#')
	{
		if (_channels.find(target) == _channels.end())
			return sendNumeric(fd, "403", target + " :No such channel");

		Channel &chan = _channels[target];
		if (!chan.hasMember(fd))
			return sendNumeric(fd, "404", target + " :Cannot send to channel");

		broadcastToChannel(target, prefix + " PRIVMSG " + target + " :" + text, fd);
	}
	else
	{
		Client *targetClient = findClientByNick(target);
		if (!targetClient)
			return sendNumeric(fd, "401", target + " :No such nick/channel");
		sendTo(targetClient->getFd(), prefix + " PRIVMSG " + target + " :" + text);
	}
}
