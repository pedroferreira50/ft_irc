#include "commands.hpp"

void	cmd_part(Server& srv, Client& client, const std::vector<std::string>& params)
{
	/* verificacoes */
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PART :Not enough parameters")));

	Channel*	channel = srv.getChannel(params[0]);
	if (!channel)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel")));
	if (!channel->isMember(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[0] + " :Not channel member")));

	/* anuncio de saida, com ou sem mensagem do usuario */
	std::string partMsg = "";
	if (params.size() > 1)
		partMsg = ":" + client.getPrefix() + " PART " + channel->getName() + " :" + params[1] + "\r\n";
	else
		partMsg = ":" + client.getPrefix() + " PART " + channel->getName() + "\r\n";
	channel->broadcastAll(srv, partMsg);

	/* remover do canal e deleta se for o ultimo membro */
	channel->removeMember(client);
	client.leaveChannel(channel->getNameLower());
	srv.removeEmptyChannel(channel->getNameLower());

}
