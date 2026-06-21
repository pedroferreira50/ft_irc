#include "commands.hpp"

/* @TODO: o cliente pode passar uma lista de canais e uma lista de membros, e nesse caso tenho 3 opcoes:
[implementado] 1 canal e 1 ou varios membros = percorrer e kickar todos
mesma quantidade de canais e membros = percorrer e kickar os pares por ordem exata
quantidade diferente de canais e membros = retorna falta de parametros ERR_NEEDMOREPARAMS (mais facil) 
*/
/* KICK #canal nick :motivo opcional */
void	cmd_kick(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (params.size() < 2)
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters")));
	Channel*	channel = srv.getChannel(params[0]);
	if (!channel)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel")));
	if (!channel->isMember(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[0] + " :Not channel member")));
	if (!channel->isOperator(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, params[0] + " :Not channel operator")));

	std::string	reason = params.size() > 2 ? params[2] : client.getNick();
	std::vector<std::string> targets = split(params[1], ',');
	for (size_t i = 0; i < targets.size(); ++i)
	{
		Client* target = srv.getClientByNick(targets[i]);
		if (!target)
		{
			srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, targets[i] + " :No such nick"));
			continue ;
		}
		if (!channel->isMember(*target))
		{
			srv.sendMsg(client, reply(srv, client, ERR_USERNOTINCHANNEL, targets[i] + " " + channel->getName() + " :They aren't on that channel"));
			continue ;
		}
		std::string kickMsg = ":" + client.getPrefix() + " KICK " + channel->getName() + " " + targets[i] + " :" + reason + "\r\n";
		channel->broadcastAll(srv, kickMsg);
		/* remocao */
		channel->removeMember(*target);
		target->leaveChannel(channel->getNameLower());
		srv.removeEmptyChannel(channel->getNameLower());
	}	
}
