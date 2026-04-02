#include "Server.hpp"

void Server::cmdWho(int fd, const Message &msg)
{
	if (msg.params.empty())
	{
		sendNumeric(fd, "315", "* :End of /WHO list");
		return;
	}

	std::string target = msg.params[0];

	if (target[0] == '#' && _channels.find(target) != _channels.end())
	{
		Channel &chan = _channels[target];
		const std::set<int> &members = chan.getMembers();
		for (std::set<int>::const_iterator it = members.begin();
			 it != members.end(); ++it)
		{
			Client &c = _clients[*it];
			std::string flags = "H";
			if (chan.isOperator(*it)) flags += "@";
			sendNumeric(fd, "352", target + " " + c.getUsername()
				+ " localhost " SERVER_NAME " " + c.getNickname()
				+ " " + flags + " :0 " + c.getRealname());
		}
	}
	sendNumeric(fd, "315", target + " :End of /WHO list");
}
