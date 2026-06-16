#include "Server.hpp"
#include "commands/commands.hpp"
#include "Channel.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <cerrno>

bool	Server::_running = true;

static void	signalHandler(int signal)
{
	(void)signal;
	Server::_running = false;
	std::cout << std::endl;
}

Server::Server(int port, const std::string& password)
	: _fd(-1), _port(port), _password(password), _serverName("irc.local")
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);
	_createSocket();
	_initCommands();
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete (it->second);
	close(_fd);
}

// ── socket setup ──────────────────────────────────────────────────────────────

void Server::_createSocket()
{
	int                opt;
	struct sockaddr_in addr;
	struct pollfd      pfd;

	opt = 1;
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw std::runtime_error("socket() failed");

	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");

	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(_port);

	if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_fd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");

	pfd.fd      = _fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);

	std::cout << "Server listening on port " << _port << std::endl;
}

// ── command registration ──────────────────────────────────────────────────────

void Server::_initCommands()
{
	_commands["PASS"] = &cmd_pass;
	_commands["NICK"] = &cmd_nick;
	_commands["USER"] = &cmd_user;
	_commands["PING"] = &cmd_ping;
	_commands["JOIN"] = &cmd_join;
	_commands["PART"] = &cmd_part;
	_commands["PRIVMSG"] = &cmd_privmsg;
	_commands["QUIT"] = &cmd_quit;
	_commands["MODE"] = &cmd_mode;
	_commands["INVITE"] = &cmd_invite;
}

// ── event loop ────────────────────────────────────────────────────────────────

void Server::run()
{
	while (_running)
	{
		int ready;

		ready = poll(&_pollfds[0], _pollfds.size(), -1);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll() failed");
		}

		for (int i = 0; i < (int)_pollfds.size(); ++i)
		{
			if (_pollfds[i].revents == 0)
				continue;

			if (_pollfds[i].fd == _fd)
			{
				_acceptClient();
			}
			else
			{
				if (_pollfds[i].revents & (POLLERR | POLLHUP))
				{
					_removeClient(_pollfds[i].fd);
					--i;
					continue;
				}
				if (_pollfds[i].revents & POLLIN)
				{
					bool removed = _readClient(_pollfds[i].fd);
					if (removed) { --i; continue; }
				}
				if (_pollfds[i].revents & POLLOUT)
				{
					bool removed = _writeClient(_pollfds[i].fd);
					if (removed) { --i; continue; }
				}
			}
		}
	}
}

// ── connection handling ───────────────────────────────────────────────────────

void Server::_acceptClient()
{
	struct sockaddr_in addr;
	socklen_t          addrlen;
	int                clientFd;
	struct pollfd      pfd;

	addrlen  = sizeof(addr);
	clientFd = accept(_fd, (struct sockaddr*)&addr, &addrlen);
	if (clientFd < 0)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl() on client failed" << std::endl;
		close(clientFd);
		return;
	}

	pfd.fd      = clientFd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	_clients[clientFd] = new Client(clientFd, inet_ntoa(addr.sin_addr));

	std::cout << "[+] Client connected: fd=" << clientFd
	          << " ip=" << inet_ntoa(addr.sin_addr) << std::endl;
	/* mensagem de instrucoes */
	std::string instructions = 
	    ":" + _serverName + " NOTICE Auth :*** To get started, please set PASS, USER and NICK.\r\n"
	    ":" + _serverName + " NOTICE Auth :*** Use 'PASS <ServerPass>' \r\n"
	    ":" + _serverName + " NOTICE Auth :*** Use 'USER <YourName> 0 * :<RealName>' \r\n"
	    ":" + _serverName + " NOTICE Auth :*** Use 'NICK <YourNickname>'\r\n";
	sendMsg(*_clients[clientFd], instructions);
}

bool Server::_readClient(int fd)
{
	char         buf[512];
	int          n;
	std::string  line;
	Message      msg;
	size_t       pos;
	std::string& rbuf = _clients[fd]->getReadBuf();

	n = recv(fd, buf, sizeof(buf), 0);
	if (n <= 0)
	{
		if (n < 0)
			std::cerr << "recv() error on fd=" << fd << std::endl;
		_removeClient(fd);
		return true;
	}

	rbuf.append(buf, n);

	while ((pos = rbuf.find('\n')) != std::string::npos)
	{
		line = rbuf.substr(0, pos);
		rbuf.erase(0, pos + 1);

		msg = Parser::parse(line);
		if (!msg.command.empty())
			_dispatch(*_clients[fd], msg);

		if (_clients.find(fd) == _clients.end())
			return true;
	}
	return false;
}

bool Server::_writeClient(int fd)
{
	Client*      client = _clients[fd];
	std::string& wbuf   = client->getWriteBuf();
	int          n;

	if (wbuf.empty())
	{
		_setPollEvent(fd, POLLOUT, false);
		return false;
	}

	n = send(fd, wbuf.c_str(), wbuf.size(), 0);
	if (n <= 0)
	{
		_removeClient(fd);
		return true;
	}
	wbuf.erase(0, n);
	if (wbuf.empty())
		_setPollEvent(fd, POLLOUT, false);
	return false;
}

void Server::_removeClient(int fd)
{
	/* @QUESTION: porque "EOF from client" ?*/
	std::string quitMsg = ":" + _clients[fd]->getPrefix() + " QUIT :EOF from client\r\n";
	std::set<std::string> channels = _clients[fd]->getChannelList();
	for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel* channel = getChannel(*it);
		if (!channel)
			continue;
		channel->broadcast(*this, *_clients[fd], quitMsg);
		channel->removeMember(*_clients[fd]);
		removeEmptyChannel(*it);
	}

	const std::string& wbuf = _clients[fd]->getWriteBuf();

	if (!wbuf.empty())
		send(fd, wbuf.c_str(), wbuf.size(), 0);

	close(fd);
	delete _clients[fd];
	_clients.erase(fd);

	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}
	std::cout << "[-] Client disconnected: fd=" << fd << std::endl;
}

void Server::_setPollEvent(int fd, short event, bool on)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			if (on)
				_pollfds[i].events |= event;
			else
				_pollfds[i].events &= ~event;
			return;
		}
	}
}

// ── command dispatch ──────────────────────────────────────────────────────────

void Server::_dispatch(Client& client, const Message& msg)
{
	static const std::string         preReg[] = {"PASS", "NICK", "USER", "PING", ""};
	std::map<std::string, CmdFn>::iterator it;
	std::string                       nick;
	bool                              requiresReg;
	int                               i;

	if (msg.command == "CAP")
		return;

	it = _commands.find(msg.command);
	if (it == _commands.end())
	{
		if (client.isRegistered())
			sendMsg(client, ":" + _serverName + " " + ERR_UNKNOWNCOMMAND + " "
			        + client.getNick() + " " + msg.command + " :Unknown command\r\n");
		return;
	}

	requiresReg = true;
	for (i = 0; !preReg[i].empty(); ++i)
	{
		if (msg.command == preReg[i])
		{
			requiresReg = false;
			break;
		}
	}

	if (requiresReg && !client.isRegistered())
	{
		nick = client.getNick().empty() ? "*" : client.getNick();
		sendMsg(client, ":" + _serverName + " " + ERR_NOTREGISTERED + " "
		        + nick + " :You have not registered\r\n");
		return;
	}

	it->second(*this, client, msg.params);
}

// ── public helpers for command handlers ──────────────────────────────────────

void Server::sendMsg(Client& client, const std::string& msg)
{
	client.getWriteBuf() += msg;
	_setPollEvent(client.getFd(), POLLOUT, true);
}
//toLower esta ser usado para nicks serem o mesmo idependente se e maiusculo ou nao
Client* Server::getClientByNick(const std::string& nick) const
{
	for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
		if (Channel::toLower(it->second->getNick()) == Channel::toLower(nick))
			return it->second;
	return NULL;
}

void Server::checkRegistration(Client& client)
{
	if (client.isRegistered() || !client.isAuthenticated()
	    || client.getNick().empty() || client.getUser().empty())
		return;

	client.setRegistered(true);
	sendMsg(client, ":" + _serverName + " " + RPL_WELCOME + " " + client.getNick()
	        + " :Welcome to the IRC network " + client.getPrefix() + "\r\n");
	sendMsg(client, ":" + _serverName + " " + RPL_YOURHOST + " " + client.getNick()
	        + " :Your host is " + _serverName + "\r\n");
}

void Server::disconnectClient(Client& client)
{
	_removeClient(client.getFd());
}

const std::string& Server::getPassword() const
{
	return _password;
}

const std::string& Server::getServerName() const
{
	return _serverName;
}

Channel*	Server::getChannel(const std::string& name) const
{
	std::map<std::string, Channel*>::const_iterator it = _channels.find(Channel::toLower(name));
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel*	Server::getOrCreateChannel(const std::string& name)
{
	std::string lower = Channel::toLower(name);
	if (_channels.find(lower) == _channels.end())
		_channels[lower] = new Channel(name);
	return (_channels[lower]);
}

void	Server::removeEmptyChannel(const std::string& nameLower)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(nameLower);
	if (it != _channels.end() && it->second->isEmpty())
	{
		delete (it->second);
		_channels.erase(it);
	}
}

