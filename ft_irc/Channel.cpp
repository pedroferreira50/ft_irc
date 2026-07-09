#include "Channel.hpp"
#include "Server.hpp"
#include <sstream>

Channel::Channel(const std::string& name)
	:	_name(name), _nameLower(toLower(name)),
		_limit(0), _inviteOnly(false), _topicLocked(false)
{
}

Channel::~Channel()
{
}

/* getters */
const std::string& Channel::getName() const
{
	return (_name);
}

const std::string& Channel::getNameLower() const
{
	return (_nameLower);
}

const std::string& Channel::getTopic() const
{
	return (_topic);
}

const std::string& Channel::getKey() const
{
	return (_key);
}

int	Channel::getLimit() const
{
	return (_limit);
}

const std::map<Client*, bool>& Channel::getMembers() const
{
	return (_members);
}

int	Channel::getMemberCount() const
{
	return (static_cast<int>(_members.size()));
}

std::string Channel::getMemberList() const
{
	std::string list;

	for (std::map<Client*, bool>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (!it->second)
			continue ;
		if (!list.empty())
			list += " ";
		list += "@" + it->first->getNick();
	}

	for (std::map<Client*, bool>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->second)
			continue ;
		if (!list.empty())
			list += " ";
		list += it->first->getNick();
	}
	return (list);
}

std::string Channel::getModeString() const
{
	std::string	modes = "+";
	std::string params;

	if (_inviteOnly)
		modes += "i";
	if (_topicLocked)
		modes += "t";
	if (!_key.empty())
	{
		modes += "k";
		params += " " +_key;
	}
	if (_limit > 0)
	{
		modes += "l";
		std::stringstream ss;
		ss << _limit;
		std::string limit = ss.str();
		params += " " + limit;
	}
	return (modes + params);
}

/* setters */
void	Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

void	Channel::setKey(const std::string& key)
{
	_key = key;
}

void	Channel::setLimit(int limit)
{
	_limit = limit;
}

void	Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

void	Channel::setTopicLocked(bool value)
{
	_topicLocked = value;
}
void	Channel::setOperator(Client& client, bool value)
{
	std::map<Client*, bool>::iterator it = _members.find(&client);
	if (it != _members.end())
		it->second = value;
}

/* members */
bool	Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

bool	Channel::isTopicLocked() const
{
	return (_topicLocked);
}

void	Channel::addMember(Client& client, bool isOperator)
{
	_members[&client] = isOperator;
}

void	Channel::removeMember(Client& client)
{
	_members.erase(&client);
}

bool	Channel::isMember(Client& client) const
{
	return (_members.find(const_cast<Client*>(&client)) != _members.end());
}

bool	Channel::isOperator(Client& client) const
{
	std::map<Client*, bool>::const_iterator it = _members.find(const_cast<Client*>(&client));
	if (it == _members.end())
		return (false);
	return (it->second);
}

bool	Channel::isEmpty() const
{
	return (_members.empty());
}

bool	Channel::isInvited(const std::string& nick) const
{
	std::string lower = toLower(nick);
	for (size_t i = 0; i < _invited.size(); ++i)
	{
		if (_invited[i] == lower)
			return (true);
	}
	return (false);
}

void	Channel::addInvited(const std::string& nick)
{
	std::string lower = toLower(nick);
	for (size_t i = 0; i < _invited.size(); ++i)
	{
		if (_invited[i] == lower)
			return ;
	}
	_invited.push_back(lower);
}

void	Channel::removeInvited(const std::string& nick)
{
	std::string lower = toLower(nick);
	for (std::vector<std::string>::iterator it = _invited.begin(); it != _invited.end(); ++it)
	{
		if (*it == lower)
		{
			_invited.erase(it);
			return ;
		}
	}
}

bool	Channel::isValidName(const std::string& name)
{
	if (name.empty() || name.size() <= 1 || name.size() > 50 || (name[0] != '#' && name[0] != '&'))	
		return (false);
	for (size_t i = 0; i < name.size(); ++i)
	{
		char c = name[i];
		if (c == ' ' || c == ',' || c == '\0' || c == '\7')
			return (false);
	}
	return (true);
}

std::string	Channel::toLower(const std::string& s)
{
	std::string	out = s;
	for (size_t i = 0; i < out.size(); ++i)
		out[i] = std::tolower(out[i]);
	return (out);	
}

/* messages */
void	Channel::broadcast(Server& server, Client& sender, const std::string& msg)
{
	for (std::map<Client*, bool>::iterator  it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->first != &sender)
			server.sendMsg(*it->first, msg);
	}
}

void	Channel::broadcastAll(Server& server, const std::string& msg)
{
	for (std::map<Client*, bool>::iterator it = _members.begin(); it != _members.end(); ++it)
		server.sendMsg(*it->first, msg);
}
