#include "Server.hpp"
#include <cstdlib>
#include <sstream>

void Server::cmdMode(int fd, const Message &msg)
{
	if (msg.params.empty())
		return sendNumeric(fd, "461", "MODE :Not enough parameters");

	std::string target = msg.params[0];

	if (target[0] != '#')
	{
		if (target != _clients[fd].getNickname())
			return sendNumeric(fd, "502", ":Can't change mode for other users");
		sendNumeric(fd, "221", "+");
		return;
	}

	if (_channels.find(target) == _channels.end())
		return sendNumeric(fd, "403", target + " :No such channel");

	Channel &chan = _channels[target];

	if (msg.params.size() == 1)
	{
		sendNumeric(fd, "324", target + " " + chan.getModeString());
		return;
	}

	if (!chan.hasMember(fd))
		return sendNumeric(fd, "442", target + " :You're not on that channel");
	if (!chan.isOperator(fd))
		return sendNumeric(fd, "482", target + " :You're not channel operator");

	std::string modeStr = msg.params[1];
	size_t      paramIdx = 2;
	bool        adding = true;

	std::string appliedModes;
	std::string appliedParams;
	bool        lastAdding = true;
	bool        firstMode = true;

	for (size_t i = 0; i < modeStr.size(); ++i)
	{
		char c = modeStr[i];
		if (c == '+') { adding = true; continue; }
		if (c == '-') { adding = false; continue; }

		bool        applied = false;
		std::string modeParam;

		switch (c)
		{
		case 'i':
			if (chan.isInviteOnly() != adding)
			{
				chan.setInviteOnly(adding);
				applied = true;
			}
			break;

		case 't':
			if (chan.isTopicLocked() != adding)
			{
				chan.setTopicLock(adding);
				applied = true;
			}
			break;

		case 'k':
			if (adding)
			{
				if (paramIdx >= msg.params.size())
				{ sendNumeric(fd, "461", "MODE :Not enough parameters"); continue; }
				chan.setKey(msg.params[paramIdx]);
				modeParam = msg.params[paramIdx];
				++paramIdx;
				applied = true;
			}
			else
			{
				if (!chan.getKey().empty())
				{
					chan.setKey("");
					applied = true;
				}
			}
			break;

		case 'o':
		{
			if (paramIdx >= msg.params.size())
			{ sendNumeric(fd, "461", "MODE :Not enough parameters"); continue; }
			std::string nick = msg.params[paramIdx];
			++paramIdx;
			Client *tc = findClientByNick(nick);
			if (!tc || !chan.hasMember(tc->getFd()))
			{ sendNumeric(fd, "441", nick + " " + target + " :They aren't on that channel"); continue; }
			if (adding)
				chan.addOperator(tc->getFd());
			else
				chan.removeOperator(tc->getFd());
			modeParam = nick;
			applied = true;
			break;
		}

		case 'l':
			if (adding)
			{
				if (paramIdx >= msg.params.size())
				{ sendNumeric(fd, "461", "MODE :Not enough parameters"); continue; }
				int limit = std::atoi(msg.params[paramIdx].c_str());
				++paramIdx;
				if (limit <= 0) continue;
				chan.setUserLimit(limit);
				std::ostringstream oss;
				oss << limit;
				modeParam = oss.str();
				applied = true;
			}
			else
			{
				if (chan.getUserLimit() > 0)
				{
					chan.setUserLimit(0);
					applied = true;
				}
			}
			break;

		default:
			sendNumeric(fd, "472", std::string(1, c) + " :is unknown mode char to me");
			break;
		}

		if (applied)
		{
			if (firstMode || adding != lastAdding)
			{
				appliedModes += adding ? '+' : '-';
				lastAdding = adding;
				firstMode = false;
			}
			appliedModes += c;
			if (!modeParam.empty())
				appliedParams += " " + modeParam;
		}
	}

	if (!appliedModes.empty())
	{
		std::string broadcast = ":" + _clients[fd].getPrefix()
			+ " MODE " + target + " " + appliedModes + appliedParams;
		const std::set<int> &members = chan.getMembers();
		for (std::set<int>::const_iterator it = members.begin();
			 it != members.end(); ++it)
			sendTo(*it, broadcast);
	}
}
