#include "Channel.hpp"
#include <sstream>

/* ================= CONSTRUCTORS ================= */

Channel::Channel()
	: _inviteOnly(false), _topicLock(false), _userLimit(0) {}

Channel::Channel(const std::string &name)
	: _name(name), _inviteOnly(false), _topicLock(false), _userLimit(0) {}

Channel::~Channel() {}

/* ================= GETTERS ================= */

const std::string &Channel::getName() const { return _name; }
const std::set<int> &Channel::getMembers() const { return _members; }
const std::string &Channel::getTopic() const { return _topic; }
bool Channel::isInviteOnly() const { return _inviteOnly; }
bool Channel::isTopicLocked() const { return _topicLock; }
const std::string &Channel::getKey() const { return _key; }
size_t Channel::getUserLimit() const { return _userLimit; }

/* ================= MEMBERS ================= */

void Channel::addMember(int fd)
{
	_members.insert(fd);
	if (_members.size() == 1)
		_operators.insert(fd);
	_invited.erase(fd);
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
}

bool Channel::hasMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

bool Channel::empty() const { return _members.empty(); }
size_t Channel::size() const { return _members.size(); }

/* ================= OPERATORS ================= */

void Channel::addOperator(int fd)
{
	if (hasMember(fd))
		_operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool Channel::isOperator(int fd) const
{
	return _operators.find(fd) != _operators.end();
}

/* ================= INVITED ================= */

void Channel::addInvited(int fd) { _invited.insert(fd); }
void Channel::removeInvited(int fd) { _invited.erase(fd); }
bool Channel::isInvited(int fd) const
{
	return _invited.find(fd) != _invited.end();
}

/* ================= TOPIC ================= */

void Channel::setTopic(const std::string &topic) { _topic = topic; }

/* ================= MODES ================= */

void Channel::setInviteOnly(bool mode) { _inviteOnly = mode; }
void Channel::setTopicLock(bool mode) { _topicLock = mode; }
void Channel::setKey(const std::string &key) { _key = key; }
void Channel::setUserLimit(size_t limit) { _userLimit = limit; }

std::string Channel::getModeString() const
{
	std::string modes = "+";
	std::string params;

	if (_inviteOnly) modes += "i";
	if (_topicLock) modes += "t";
	if (!_key.empty())
	{
		modes += "k";
		params += " " + _key;
	}
	if (_userLimit > 0)
	{
		modes += "l";
		std::ostringstream oss;
		oss << _userLimit;
		params += " " + oss.str();
	}
	if (modes == "+")
		return "+";
	return modes + params;
}
