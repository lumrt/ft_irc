#include "Server.hpp"

void Server::cmdTopic(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "461", "TOPIC :Not enough parameters");

	std::string chanName = msg.params[0];

	if (_channels.find(chanName) == _channels.end())
		return sendNumeric(fd, "403", chanName + " :No such channel");

	Channel &chan = _channels[chanName];

	if (!chan.hasMember(fd))
		return sendNumeric(fd, "442", chanName + " :You're not on that channel");

	if (msg.params.size() == 1)
	{
		if (chan.getTopic().empty())
			return sendNumeric(fd, "331", chanName + " :No topic is set");
		return sendNumeric(fd, "332", chanName + " :" + chan.getTopic());
	}

	if (chan.isTopicLocked() && !chan.isOperator(fd))
		return sendNumeric(fd, "482", chanName + " :You're not channel operator");

	chan.setTopic(msg.params[1]);

	std::string topicMsg = ":" + _clients[fd].getPrefix()
		+ " TOPIC " + chanName + " :" + msg.params[1];
	broadcastToChannel(chanName, topicMsg, -1);
}
