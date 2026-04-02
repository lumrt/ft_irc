#include "Server.hpp"

static bool isValidChannelName(const std::string &name)
{
	if (name.size() < 2 || name[0] != '#' || name.size() > 50)
		return false;
	for (size_t i = 1; i < name.size(); ++i)
	{
		if (name[i] == ' ' || name[i] == ',' || name[i] == '\x07')
			return false;
	}
	return true;
}

void Server::cmdJoin(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "461", "JOIN :Not enough parameters");

	if (msg.params[0] == "0")
	{
		for (std::map<std::string, Channel>::iterator it = _channels.begin();
			 it != _channels.end(); )
		{
			if (it->second.hasMember(fd))
			{
				std::string partMsg = ":" + _clients[fd].getPrefix() + " PART " + it->first;
				broadcastToChannel(it->first, partMsg, -1);
				it->second.removeMember(fd);
			}
			if (it->second.empty())
				_channels.erase(it++);
			else
				++it;
		}
		return;
	}

	std::vector<std::string> channels = splitList(msg.params[0], ',');
	std::vector<std::string> keys;
	if (msg.params.size() > 1)
		keys = splitList(msg.params[1], ',');

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];
		std::string key = (i < keys.size()) ? keys[i] : "";

		if (!isValidChannelName(chanName))
		{
			sendNumeric(fd, "403", chanName + " :No such channel");
			continue;
		}

		bool newChannel = (_channels.find(chanName) == _channels.end());

		if (!newChannel)
		{
			Channel &chan = _channels[chanName];

			if (chan.hasMember(fd))
				continue;
			if (chan.isInviteOnly() && !chan.isInvited(fd))
			{
				sendNumeric(fd, "473", chanName + " :Cannot join channel (+i)");
				continue;
			}
			if (!chan.getKey().empty() && chan.getKey() != key)
			{
				sendNumeric(fd, "475", chanName + " :Cannot join channel (+k)");
				continue;
			}
			if (chan.getUserLimit() > 0 && chan.size() >= chan.getUserLimit())
			{
				sendNumeric(fd, "471", chanName + " :Cannot join channel (+l)");
				continue;
			}
		}

		if (newChannel)
			_channels.insert(std::make_pair(chanName, Channel(chanName)));

		_channels[chanName].addMember(fd);

		std::string joinMsg = ":" + _clients[fd].getPrefix() + " JOIN " + chanName;
		broadcastToChannel(chanName, joinMsg, -1);

		sendChannelInfo(fd, _channels[chanName]);
	}
}
