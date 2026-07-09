#include "commands.hpp"
#include <iostream>

void	cmd_join(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters")));

	std::vector<std::string> chans = split(params[0], ',');
	std::vector<std::string> keys = params.size() > 1 ? split(params[1], ',') : std::vector<std::string>();

	for (size_t i = 0; i < chans.size(); ++i)
	{
		std::string	password = i < keys.size() ? keys[i] : "";

		if (!Channel::isValidName(chans[i]))
		{
			srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, chans[i] + " :No such channel"));
			continue ;
		}

		Channel*	channel = srv.getChannel(chans[i]);

		if (channel)
		{
			if (channel->isMember(client))
				continue ;
			if (!channel->getKey().empty() && channel->getKey() != password)
			{
				srv.sendMsg(client, reply(srv, client, ERR_BADCHANNELKEY, chans[i] + " :Cannot join channel (+k)"));
				continue ;
			}
			if (channel->isInviteOnly() && !channel->isInvited(client.getNick()))
			{
				srv.sendMsg(client, reply(srv, client, ERR_INVITEONLYCHAN, chans[i] + " :Cannot join channel (+i)"));
				continue ;
			}
			if (channel->getLimit() > 0 && channel->getMemberCount() >= channel->getLimit())
			{	
				srv.sendMsg(client, reply(srv, client, ERR_CHANNELISFULL, chans[i] + " :Cannot join channel (+l)"));
				continue ;
			}
		}
		if (!channel)
			channel = srv.getOrCreateChannel(chans[i]);

		channel->addMember(client, channel->isEmpty());
		client.joinChannel(channel->getNameLower());
		channel->removeInvited(client.getNick());

		const std::string joinMsg = ":" + client.getPrefix() + " JOIN " + channel->getName() + "\r\n";
		channel->broadcastAll(srv, joinMsg);
		if (channel->getTopic().empty())
			srv.sendMsg(client, reply(srv, client, RPL_NOTOPIC, channel->getName() + " :No topic is set"));
		else
			srv.sendMsg(client, reply(srv, client, RPL_TOPIC, channel->getName() + " :" + channel->getTopic()));

		srv.sendMsg(client, reply(srv, client, RPL_NAMREPLY, "= " + channel->getName() + " :" + channel->getMemberList()));
		srv.sendMsg(client, reply(srv, client, RPL_ENDOFNAMES, channel->getName() + " :End of /NAMES list"));
	}
}
