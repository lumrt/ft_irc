#include "Server.hpp"

// ERR_NEEDMOREPARAMS (461)
// ERR_ALREADYREGISTRED (462)
// ERR_PASSWDMISMATCH (464)

void Server::cmdPass(int fd, const Message &msg)
{
	Client &client = _clients[fd];

	if (client.isRegistered())
		return sendNumeric(fd, "462", ":You may not reregister");
	if (msg.params.empty())
		return sendNumeric(fd, "461", "PASS :Not enough parameters");
	if (msg.params[0] != _password)
		return sendNumeric(fd, "464", ":Password incorrect");

	client.setPassAccepted(true);
}
