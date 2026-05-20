#include "commands.hpp"

void cmd_user(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (client.isRegistered())
		return srv.sendMsg(client, reply(srv, client, ERR_ALREADYREGISTRED, ":You may not reregister"));
	if (params.size() < 4)
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters"));

	client.setUser(params[0]);
	client.setRealname(params[3]);
	srv.checkRegistration(client);
}
