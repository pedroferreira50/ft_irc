#include "Client.hpp"

Client::Client(int fd, const std::string& hostname)
	: _fd(fd), _hostname(hostname),
	  _authenticated(false), _registered(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getHostname() const
{
	return _hostname;
}

std::string Client::getPrefix() const
{
	return _nick + "!" + _user + "@" + _hostname;
}

const std::string& Client::getNick() const
{
	return _nick;
}

const std::string& Client::getUser() const
{
	return _user;
}

const std::string& Client::getRealname() const
{
	return _realname;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setNick(const std::string& nick)
{
	_nick = nick;
}

void Client::setUser(const std::string& user)
{
	_user = user;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

void Client::setAuthenticated(bool val)
{
	_authenticated = val;
}

void Client::setRegistered(bool val)
{
	_registered = val;
}

std::string& Client::getReadBuf()
{
	return _readBuf;
}

std::string& Client::getWriteBuf()
{
	return _writeBuf;
}

const std::set<std::string>& Client::getChannelList() const
{
	return (_channelList);
}

void	Client::joinChannel(const std::string& nameLower)
{
	_channelList.insert(nameLower);
}

void	Client::leaveChannel(const std::string& nameLower)
{
	_channelList.erase(nameLower);
}

bool	Client::isInChannel(const std::string& nameLower) const
{
	return (_channelList.find(nameLower) != _channelList.end());
}
