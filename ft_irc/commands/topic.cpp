#include "commands.hpp"
#include <cstdlib>

void    cmd_topic(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters")));

	Channel* channel = srv.getChannel(params[0]);
	if (!channel)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel")));
	if (!channel->isMember(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[0] + " :You're not on that channel")));

	if (params.size() == 1)
	{
		if (channel->getTopic().empty())
			return (srv.sendMsg(client, reply(srv, client, RPL_NOTOPIC, channel->getName() + " :No topic is set")));
		return (srv.sendMsg(client, reply(srv, client, RPL_TOPIC, channel->getName() + " :" + channel->getTopic())));
	}

	if (channel->isTopicLocked() && !channel->isOperator(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, channel->getName() + " :You're not channel operator")));

	channel->setTopic(params[1]);
	std::string topicMsg = ":" + client.getPrefix() + " TOPIC " + channel->getName() + " :" + params[1] + "\r\n";
	channel->broadcastAll(srv, topicMsg);
}