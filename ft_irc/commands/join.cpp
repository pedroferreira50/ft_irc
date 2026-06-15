#include "commands.hpp"
#include <iostream>

void	cmd_join(Server& srv, Client& client, const std::vector<std::string>& params)
{
	/* verificacoes */
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters")));

	//divide params[0] e params[1] para o loop conseguir entrar em varias channels
	std::vector<std::string> chans = split(params[0], ',');
	std::vector<std::string> keys = params.size() > 1 ? split(params[1], ',') : std::vector<std::string>();

	for (size_t i = 0; i < chans.size(); ++i)
	{
		std::string	password = i < keys.size() ? keys[i] : "";
		//std::cout << "DEBUG channel: " << chans[i] << " password: '" << password << "'" << std::endl;

		if (!Channel::isValidName(chans[i]))
		{
			srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, chans[i] + " :No such channel"));
			continue ;
		}
	
		Channel*	channel = srv.getChannel(chans[i]);

		/* outras verificacoes */
		if (channel)
		{
			if (channel->isMember(client)) /* membro */
				continue ;
			if (!channel->getKey().empty() && channel->getKey() != password) /* senha */
			{
				srv.sendMsg(client, reply(srv, client, ERR_BADCHANNELKEY, chans[i] + " :Cannot join channel (+k)"));
				continue ;
			}
			if (channel->isInviteOnly() && !channel->isInvited(client.getNick())) /* modo invite only */
			{
				srv.sendMsg(client, reply(srv, client, ERR_INVITEONLYCHAN, chans[i] + " :Cannot join channel (+i)"));
				continue ;
			}
			if (channel->getLimit() > 0 && channel->getMemberCount() >= channel->getLimit()) /* limite de membros */
			{	
				srv.sendMsg(client, reply(srv, client, ERR_CHANNELISFULL, chans[i] + " :Cannot join channel (+l)"));
				continue ;
			}
		}
		if (!channel)
			channel = srv.getOrCreateChannel(chans[i]);
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
}
