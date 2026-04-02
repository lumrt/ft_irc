#include "Server.hpp"

void Server::cmdKick(int fd, const Message &msg)
{
	if (msg.params.size() < 2)
		return sendNumeric(fd, "461", "KICK :Not enough parameters");

	std::string chanName = msg.params[0];
	std::string targetNick = msg.params[1];
	std::string reason = msg.params.size() > 2 ? msg.params[2] : targetNick;

	if (_channels.find(chanName) == _channels.end())
		return sendNumeric(fd, "403", chanName + " :No such channel");

	Channel &chan = _channels[chanName];

	if (!chan.hasMember(fd))
		return sendNumeric(fd, "442", chanName + " :You're not on that channel");
	if (!chan.isOperator(fd))
		return sendNumeric(fd, "482", chanName + " :You're not channel operator");

	Client *targetClient = findClientByNick(targetNick);
	if (!targetClient || !chan.hasMember(targetClient->getFd()))
		return sendNumeric(fd, "441", targetNick + " " + chanName + " :They aren't on that channel");

	std::string kickMsg = ":" + _clients[fd].getPrefix()
		+ " KICK " + chanName + " " + targetNick + " :" + reason;
	broadcastToChannel(chanName, kickMsg, -1);

	chan.removeMember(targetClient->getFd());
	if (chan.empty())
		_channels.erase(chanName);
}
