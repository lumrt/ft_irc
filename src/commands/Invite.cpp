#include "Server.hpp"

void Server::cmdInvite(int fd, const Message &msg)
{
	if (msg.params.size() < 2)
		return sendNumeric(fd, "461", "INVITE :Not enough parameters");

	std::string targetNick = msg.params[0];
	std::string chanName = msg.params[1];

	Client *targetClient = findClientByNick(targetNick);
	if (!targetClient)
		return sendNumeric(fd, "401", targetNick + " :No such nick/channel");

	if (_channels.find(chanName) == _channels.end())
		return sendNumeric(fd, "403", chanName + " :No such channel");

	Channel &chan = _channels[chanName];

	if (!chan.hasMember(fd))
		return sendNumeric(fd, "442", chanName + " :You're not on that channel");

	if (chan.hasMember(targetClient->getFd()))
		return sendNumeric(fd, "443", targetNick + " " + chanName + " :is already on channel");

	if (chan.isInviteOnly() && !chan.isOperator(fd))
		return sendNumeric(fd, "482", chanName + " :You're not channel operator");

	chan.addInvited(targetClient->getFd());

	sendNumeric(fd, "341", targetNick + " " + chanName);
	sendTo(targetClient->getFd(),
		":" + _clients[fd].getPrefix() + " INVITE " + targetNick + " " + chanName);
}
