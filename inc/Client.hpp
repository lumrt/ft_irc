#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int         _fd;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _buffer;
	bool        _passAccepted;
	bool        _registered;

public:
	Client();
	Client(int fd);
	~Client();

	int                getFd() const;
	const std::string &getNickname() const;
	const std::string &getUsername() const;
	const std::string &getRealname() const;
	std::string       &getBuffer();
	bool               isPassAccepted() const;
	bool               isRegistered() const;

	void setNickname(const std::string &nickname);
	void setUsername(const std::string &username);
	void setRealname(const std::string &realname);
	void setPassAccepted(bool accepted);
	void setRegistered(bool registered);

	std::string getPrefix() const;
};

#endif
