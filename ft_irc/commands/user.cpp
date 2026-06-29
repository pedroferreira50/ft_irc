#include "commands.hpp"

static bool isValidUser(const std::string& user)
{
	if (user.empty())
		return false;
	for (size_t i = 0; i < user.size(); ++i)
	{
		unsigned char c = user[i];
		if (c == '\0' || c == '\r' || c == '\n' || c == ' ' || c == '@')
			return false;
	}
	return true;
}

void cmd_user(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (client.isRegistered())
		return srv.sendMsg(client, reply(srv, client, ERR_ALREADYREGISTRED, ":You may not reregister"));
	if (params.size() < 4)
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters"));
	if (params.size() > 4)
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Too many parameters"));
	if (!isValidUser(params[0]))
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Invalid username"));
	if (params[1] != "0")
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Second parameter must be 0"));
	if (params[2] != "*")
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Third parameter must be *"));

	client.setUser(params[0]);
	client.setRealname(params[3]);
	/* se senha for vazia e ja estiver setado o nick, autentica o cliente */
	if (srv.getPassword().empty() && !client.getNick().empty())
		client.setAuthenticated(true);
	srv.checkRegistration(client);
}
