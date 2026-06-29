#include "commands.hpp"

void cmd_pass(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (client.isRegistered())
		return srv.sendMsg(client, reply(srv, client, ERR_ALREADYREGISTRED, ":You may not reregister"));
	if (srv.getPassword().empty())
	{
		client.setAuthenticated(true);
		srv.checkRegistration(client);
		return ;
	}
	if (params.empty())
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters"));
	if (params[0] != srv.getPassword())
	{
		srv.sendMsg(client, reply(srv, client, ERR_PASSWDMISMATCH, ":Password incorrect"));
		client.setDC();
		return;
	}
	client.setAuthenticated(true);
	srv.checkRegistration(client);
}
