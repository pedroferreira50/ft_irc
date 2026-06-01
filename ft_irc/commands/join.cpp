#include "commands.hpp"

/* @QUESTION: a lista de membros de um canal pode ser por conexao no servidor ou deve ser por insercao no canal ?*/
void	cmd_join(Server& srv, Client& client, const std::vector<std::string>& params)
{
	/* verificacoes */
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters")));
	if (!Channel::isValidName(params[0]))
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel")));
	
	std::string	password = params.size() > 1 ? params[1] : "";
	Channel*	channel = srv.getChannel(params[0]);

	/* outras verificacoes */
	if (channel)
	{
		if (channel->isMember(client)) /* membro */
			return ;
		if (!channel->getKey().empty() && channel->getKey() != password) /* senha */
			return (srv.sendMsg(client, reply(srv, client, ERR_BADCHANNELKEY, params[0] + " :Cannot join channel (+k)")));
		if (channel->isInviteOnly() && !channel->isInvited(client.getNick())) /* modo invite only */
			return (srv.sendMsg(client, reply(srv, client, ERR_INVITEONLYCHAN, params[0] + " :Cannot join channel (+i)")));
		if (channel->getLimit() > 0 && channel->getMemberCount() >= channel->getLimit()) /* limite de membros */
			return (srv.sendMsg(client, reply(srv, client, ERR_CHANNELISFULL, params[0] + " :Cannot join channel (+l)")));
	}
	if (!channel)
		channel = srv.getOrCreateChannel(params[0]);
	/* adiciona membro e remove da lista de Invited (caso esteja) */
	channel->addMember(client, channel->isEmpty());
	client.joinChannel(channel->getNameLower());
	channel->removeInvited(client.getNick());

	/* mensagem */
	const std::string joinMsg = ":" + client.getPrefix() + " JOIN " + channel->getName() + "\r\n";
	channel->broadcastAll(srv, joinMsg);
	if (channel->getTopic().empty())
		srv.sendMsg(client, reply(srv, client, RPL_NOTOPIC, channel->getName() + " :No topic is set"));
	else
		srv.sendMsg(client, reply(srv, client, RPL_TOPIC, channel->getName() + " :" + channel->getTopic()));
	/* lista de membros do canal */
	srv.sendMsg(client, reply(srv, client, RPL_NAMREPLY, "= " + channel->getName() + " :" + channel->getMemberList()));
	srv.sendMsg(client, reply(srv, client, RPL_ENDOFNAMES, channel->getName() + " :End of /NAMES list"));
}
