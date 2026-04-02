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
	std::set<int> _invited;
	std::string   _topic;
	bool          _inviteOnly;
	bool          _topicLock;
	std::string   _key;
	size_t        _userLimit;

public:
	Channel();
	Channel(const std::string &name);
	~Channel();

	const std::string   &getName() const;
	const std::set<int> &getMembers() const;
	const std::string   &getTopic() const;
	bool                 isInviteOnly() const;
	bool                 isTopicLocked() const;
	const std::string   &getKey() const;
	size_t               getUserLimit() const;

	void   addMember(int fd);
	void   removeMember(int fd);
	bool   hasMember(int fd) const;
	bool   empty() const;
	size_t size() const;

	void addOperator(int fd);
	void removeOperator(int fd);
	bool isOperator(int fd) const;

	void addInvited(int fd);
	void removeInvited(int fd);
	bool isInvited(int fd) const;

	void setTopic(const std::string &topic);
	void setInviteOnly(bool mode);
	void setTopicLock(bool mode);
	void setKey(const std::string &key);
	void setUserLimit(size_t limit);

	std::string getModeString() const;
};

#endif
