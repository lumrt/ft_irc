#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel
{
private:
	std::string   _name;
	std::set<int> _members;
	std::set<int> _operators;
	std::string   _topic;

public:
	Channel();
	Channel(const std::string &name);
	~Channel();

	const std::string   &getName() const;
	const std::set<int> &getMembers() const;
	const std::string   &getTopic() const;

	void addMember(int fd);
	void removeMember(int fd);
	bool hasMember(int fd) const;
	bool empty() const;

	void addOperator(int fd);
	bool isOperator(int fd) const;

	void setTopic(const std::string &topic);
};

#endif
