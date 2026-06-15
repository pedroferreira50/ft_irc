#include "commands.hpp"
#include <cstdlib>

static void	broadcastMode(Server& srv, Client& client, Channel* channel, const std::string& modeStr, const std::string& param)
{
	std::string msg = ":" + client.getPrefix() + " MODE " + channel->getName() + " " + modeStr;
	if (!param.empty())
		msg += " " + param;
	msg += "\r\n";
	channel->broadcastAll(srv, msg);
}

static void	modeOperator(Server& srv, Client& client, Channel* channel, const std::vector<std::string>& params, bool give)
{
	if (params.size() < 3)
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));

	Client* target = srv.getClientByNick(params[2]);

	if (!target)
		return srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, params[2] + " :No such nick"));
	if (!channel->isMember(*target))
		return srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[2] + " :They aren't on that channel"));

	channel->setOperator(*target, give);
	std::string modeStr;

	if (give)
		modeStr = "+o";
	else
		modeStr = "-o";

	broadcastMode(srv, client, channel, modeStr, params[2]);
}


void	cmd_mode(Server& srv, Client& client, const std::vector<std::string>& params)
{
	
	if (params.empty())
		return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));

	Channel* channel = srv.getChannel(params[0]);
	if (!channel)
		return srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel"));

	if (params.size() == 1)
		return srv.sendMsg(client, reply(srv, client, RPL_CHANNELMODEIS, channel->getName() + " " + channel->getModeString()));

	if (channel->isOperator(client))
	{
    	if (params[1] == "+k")
		{
			if (params.size() < 3 || params[2].empty())
				return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));
			channel->setKey(params[2]);
			broadcastMode(srv, client, channel, "+k", params[2]);
		}
		else if (params[1] == "-k")
		{
			channel->setKey("");
			broadcastMode(srv, client, channel, "-k", "");
		}
		else if (params[1] == "+i")
		{
			channel->setInviteOnly(true);
			broadcastMode(srv, client, channel, "+i", "");
		}
		else if (params[1] == "-i")
		{
			channel->setInviteOnly(false);
			broadcastMode(srv, client, channel, "-i", "");
		}
		else if (params[1] == "+t")
		{
			channel->setTopicLocked(true);
			broadcastMode(srv, client, channel, "+t", "");
		}
		else if (params[1] == "-t")
		{
			channel->setTopicLocked(false);
			broadcastMode(srv, client, channel, "-t", "");
		}
		else if (params[1] == "+o")
			modeOperator(srv, client, channel, params, true);
		else if (params[1] == "-o")
			modeOperator(srv, client, channel, params, false);
		else if (params[1] == "+l")
		{
			if (params.size() < 3)
				return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));
			int limit = atoi(params[2].c_str());
			if (limit <= 0)
				return srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Invalid limit"));
			channel->setLimit(limit);
			broadcastMode(srv, client, channel, "+l", params[2]);
		}
		else if (params[1] == "-l")
		{
			channel->setLimit(0);
			broadcastMode(srv, client, channel, "-l", "");
		}


	}
	else
		srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, params[0] + " :You're not channel operator"));

}
