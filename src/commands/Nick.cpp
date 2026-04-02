#include "Server.hpp"
#include <set>

static bool isValidNick(const std::string &nick)
{
	if (nick.empty() || nick.size() > 9)
		return false;
	char c = nick[0];
	if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
		|| c == '[' || c == ']' || c == '\\' || c == '`'
		|| c == '_' || c == '^' || c == '{' || c == '|' || c == '}'))
		return false;
	for (size_t i = 1; i < nick.size(); ++i)
	{
		c = nick[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9')
			|| c == '[' || c == ']' || c == '\\' || c == '`'
			|| c == '_' || c == '^' || c == '{' || c == '|'
			|| c == '}' || c == '-'))
			return false;
	}
	return true;
}

void Server::cmdNick(int fd, const Message &msg)
{
	Client &client = _clients[fd];

	if (msg.params.empty())
		return sendNumeric(fd, "431", ":No nickname given");

	const std::string &nick = msg.params[0];

	if (!isValidNick(nick))
		return sendNumeric(fd, "432", nick + " :Erroneous nickname");

	Client *existing = findClientByNick(nick);
	if (existing && existing->getFd() != fd)
		return sendNumeric(fd, "433", nick + " :Nickname is already in use");

	std::string oldPrefix = client.getPrefix();
	std::string oldNick = client.getNickname();
	client.setNickname(nick);

	if (client.isRegistered())
	{
		std::string nickMsg = ":" + (oldNick.empty() ? nick : oldPrefix) + " NICK " + nick;

		std::set<int> notified;
		notified.insert(fd);
		for (std::map<std::string, Channel>::iterator it = _channels.begin();
			 it != _channels.end(); ++it)
		{
			if (it->second.hasMember(fd))
			{
				const std::set<int> &members = it->second.getMembers();
				notified.insert(members.begin(), members.end());
			}
		}
		for (std::set<int>::const_iterator it = notified.begin();
			 it != notified.end(); ++it)
			sendTo(*it, nickMsg);
	}
	else
		tryRegister(fd);
}
