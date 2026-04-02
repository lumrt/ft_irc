#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

struct Message
{
	std::string              prefix;
	std::string              command;
	std::vector<std::string> params;
};

Message                      parseMessage(const std::string &raw);
std::vector<std::string>     splitList(const std::string &s, char delim);

#endif
