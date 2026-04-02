#include "Message.hpp"

static std::string toUpper(const std::string &str)
{
	std::string result = str;
	for (size_t i = 0; i < result.size(); ++i)
	{
		if (result[i] >= 'a' && result[i] <= 'z')
			result[i] -= 32;
	}
	return result;
}

Message parseMessage(const std::string &raw)
{
	Message msg;
	size_t  pos = 0;

	while (pos < raw.size() && raw[pos] == ' ')
		++pos;

	if (pos < raw.size() && raw[pos] == ':')
	{
		size_t end = raw.find(' ', pos);
		if (end == std::string::npos)
			return msg;
		msg.prefix = raw.substr(pos + 1, end - pos - 1);
		pos = end + 1;
	}

	while (pos < raw.size() && raw[pos] == ' ')
		++pos;

	{
		size_t end = raw.find(' ', pos);
		if (end == std::string::npos)
		{
			msg.command = toUpper(raw.substr(pos));
			return msg;
		}
		msg.command = toUpper(raw.substr(pos, end - pos));
		pos = end + 1;
	}

	while (pos < raw.size())
	{
		while (pos < raw.size() && raw[pos] == ' ')
			++pos;
		if (pos >= raw.size())
			break;

		if (raw[pos] == ':')
		{
			msg.params.push_back(raw.substr(pos + 1));
			break;
		}

		size_t end = raw.find(' ', pos);
		if (end == std::string::npos)
		{
			msg.params.push_back(raw.substr(pos));
			break;
		}
		msg.params.push_back(raw.substr(pos, end - pos));
		pos = end + 1;
	}

	return msg;
}

std::vector<std::string> splitList(const std::string &s, char delim)
{
	std::vector<std::string> result;
	std::string::size_type   start = 0;
	std::string::size_type   end;

	while ((end = s.find(delim, start)) != std::string::npos)
	{
		result.push_back(s.substr(start, end - start));
		start = end + 1;
	}
	result.push_back(s.substr(start));
	return result;
}
