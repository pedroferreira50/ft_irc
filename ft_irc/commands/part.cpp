#include "commands.hpp"

void	cmd_part(Server& srv, Client& client, const std::vector<std::string>& params)
{
	/* verificacoes */
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PART :Not enough parameters")));

	std::vector<std::string> chans = split(params[0], ',');

	for (size_t i = 0; i < chans.size(); ++i)
	{

		Channel*	channel = srv.getChannel(chans[i]);
		if (!channel)
		{
			srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, chans[i] + " :No such channel"));
			continue ;
		}
		if (!channel->isMember(client))
		{
			srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, chans[i] + " :Not channel member"));
			continue ;
		}

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
}
