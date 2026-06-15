#include "commands.hpp"
#include <cctype>

static bool isValidNick(const std::string& nick)
{
	if (nick.empty() || nick.size() > 30)
		return false;
	if (!std::isalpha(nick[0]) && nick[0] != '_' && nick[0] != '[' && nick[0] != ']')
		return false;
	for (size_t i = 1; i < nick.size(); ++i)
	{
		if (!std::isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_')
			return false;
	}
	return true;
}

void cmd_nick(Server& srv, Client& client, const std::vector<std::string>& params)
{
	std::string newNick;
	Client*     existing;

	if (params.empty() || params[0].empty())
		return srv.sendMsg(client, reply(srv, client, ERR_NONICKNAMEGIVEN, ":No nickname given"));

	newNick = params[0];

	if (!isValidNick(newNick))
		return srv.sendMsg(client, reply(srv, client, ERR_ERRONEUSNICK, newNick + " :Erroneous nickname"));

	existing = srv.getClientByNick(newNick);
	if (existing && existing != &client)
		return srv.sendMsg(client, reply(srv, client, ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use"));

	if (client.isRegistered())
		srv.sendMsg(client, ":" + client.getPrefix() + " NICK :" + newNick + "\r\n");

	client.setNick(newNick);
	/* se senha for vazia e ja estiver setado o user, autentica o cliente */
	if (srv.getPassword().empty() && !client.getUser().empty())
		client.setAuthenticated(true);
	srv.checkRegistration(client);
}
