#include "commands.hpp"

/* @TODO: o cliente pode passar uma lista de canais e uma lista de membros, e nesse caso tenho 3 opcoes:
1 canal e 1 ou varios membros = percorrer e kickar todos
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
		return (srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, params[1] + " :Not channel operator")));

	Client* target = srv.getClientByNick(params[1]);
	if (!target)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, params[1] + " :No such nick")));
	if (!channel->isMember(*target))
		return (srv.sendMsg(client, reply(srv, client, ERR_USERNOTINCHANNEL , params[1] + " :Not channel member")));
	std::string kickMsg = "";
	if (params.size() > 2) /* motivo do kick, caso nao passado = usar algo padrao (nome de que expulsou)*/
		kickMsg = ":" + client.getPrefix() + " KICK " + channel->getName() + " " + params[1] + " :" + params[2] + "\r\n";
	else
		kickMsg = ":" + client.getPrefix() + " KICK " + channel->getName() + " " + params[1] + " :" + client.getNick() + "\r\n";
	channel->broadcastAll(srv, kickMsg);
	/* remocao */
	channel->removeMember(*target);
	target->leaveChannel(channel->getNameLower());
	srv.removeEmptyChannel(channel->getNameLower());
}
