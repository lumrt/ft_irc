#include "Server.hpp"

void Server::cmdPart(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "461", "PART :Not enough parameters");

	std::vector<std::string> channels = splitList(msg.params[0], ',');
	std::string reason = msg.params.size() > 1 ? msg.params[1] : "";

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];

		if (_channels.find(chanName) == _channels.end())
		{
			sendNumeric(fd, "403", chanName + " :No such channel");
			continue;
		}

		Channel &chan = _channels[chanName];

		if (!chan.hasMember(fd))
		{
			sendNumeric(fd, "442", chanName + " :You're not on that channel");
			continue;
		}

		std::string partMsg = ":" + _clients[fd].getPrefix() + " PART " + chanName;
		if (!reason.empty())
			partMsg += " :" + reason;
		broadcastToChannel(chanName, partMsg, -1);

		chan.removeMember(fd);
		if (chan.empty())
			_channels.erase(chanName);
	}
}
