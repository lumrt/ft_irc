#include "Channel.hpp"

/* ================= CONSTRUCTORS ================= */

Channel::Channel() {}

Channel::Channel(const std::string &name)
    : _name(name) {}

Channel::~Channel() {}

/* ================= GETTERS ================= */

const std::string &Channel::getName() const
{
    return _name;
}

const std::set<int> &Channel::getMembers() const
{
    return _members;
}

const std::string &Channel::getTopic() const
{
    return _topic;
}

/* ================= MEMBERS ================= */

void Channel::addMember(int fd)
{
    _members.insert(fd);

    // Premier membre devient opérateur automatiquement
    if (_members.size() == 1)
        _operators.insert(fd);
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

bool Channel::empty() const
{
    return _members.empty();
}

/* ================= OPERATORS ================= */

void Channel::addOperator(int fd)
{
    if (hasMember(fd))
        _operators.insert(fd);
}

bool Channel::isOperator(int fd) const
{
    return _operators.find(fd) != _operators.end();
}

/* ================= TOPIC ================= */

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}