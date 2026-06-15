#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <string>
# include <poll.h>
# include <signal.h>
# include "Client.hpp"
# include "Parser.hpp"

class Channel;

class Server
{
private:
	typedef void (*CmdFn)(Server&, Client&, const std::vector<std::string>&);

	int								_fd;
	int								_port;
	std::string						_password;
	std::string						_serverName;
	std::vector<struct pollfd>		_pollfds;
	std::map<int, Client*>			_clients;
	std::map<std::string, CmdFn>	_commands;
	std::map<std::string, Channel*>	_channels;

	void							_createSocket();
	void							_initCommands();
	void							_dispatch(Client& client, const Message& msg);
	void							_acceptClient();
	bool							_readClient(int fd);
	bool							_writeClient(int fd);
	void							_removeClient(int fd);
	void							_setPollEvent(int fd, short event, bool on);

public:
	Server(int port, const std::string& password);
	~Server();

	static bool						_running;
	void							run();
	void							sendMsg(Client& client, const std::string& msg);
	Client*							getClientByNick(const std::string& nick) const;
	void							checkRegistration(Client& client);
	void							disconnectClient(Client& client);

	const std::string&				getPassword() const;
	const std::string&				getServerName() const;
	Channel*						getChannel(const std::string& name) const;
	Channel*						getOrCreateChannel(const std::string& name);

	void							removeEmptyChannel(const std::string& nameLower);

};

#endif
