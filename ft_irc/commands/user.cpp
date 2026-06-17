#include "commands.hpp"

/* @QUESTION: nao devemos verificar se o cliente passou exatamente 'user <name> 0 * :' ?
pois da forma que esta o cliente pode passar qualquer coisa, desde que tenha USER + 4 parametros 
como por exemplo 'user 1 2 3 4' e funciona */
void cmd_user(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (client.isRegistered())
		return srv.sendMsg(client, reply(srv, client, ERR_ALREADYREGISTRED, ":You may not reregister"));
	if (params.size() < 4)
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters"));

	client.setUser(params[0]);
	client.setRealname(params[3]);
	/* se senha for vazia e ja estiver setado o nick, autentica o cliente */
	if (srv.getPassword().empty() && !client.getNick().empty())
		client.setAuthenticated(true);
	srv.checkRegistration(client);
}
