#include "commands.hpp"
#include <cstdlib>

struct ModeChange
{
	char        sign;
	char        letter;
	std::string param;
};

static std::vector<ModeChange> parseModeChanges(const std::vector<std::string>& params)
{
	std::vector<ModeChange> changes;
	const std::string&      modeStr = params[1];
	size_t                  paramIdx = 2;
	char                    sign = '+';

	for (size_t i = 0; i < modeStr.size(); ++i)
	{
		char c = modeStr[i];

		if (c == '+' || c == '-')
		{
			sign = c; 
			continue;
		}

		ModeChange mc;
		mc.sign   = sign;
		mc.letter = c;
		mc.param  = "";

		bool needsParam = (c == 'o')
		               || (c == 'k' && sign == '+')
		               || (c == 'l' && sign == '+');

		if (needsParam && paramIdx < params.size())
			mc.param = params[paramIdx++];

		changes.push_back(mc);
	}
	return changes;
}

static void	broadcastMode(Server& srv, Client& client, Channel* channel, const std::string& modeStr, const std::string& param)
{
	std::string msg = ":" + client.getPrefix() + " MODE " + channel->getName() + " " + modeStr;
	if (!param.empty())
		msg += " " + param;
	msg += "\r\n";
	channel->broadcastAll(srv, msg);
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

	if (!channel->isOperator(client))
		return srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, params[0] + " :You're not channel operator"));

	std::vector<ModeChange> changes = parseModeChanges(params);

	for (size_t i = 0; i < changes.size(); ++i)
	{
		ModeChange& mc = changes[i];

		if (mc.letter == 'i')
		{
			channel->setInviteOnly(mc.sign == '+');
			broadcastMode(srv, client, channel, std::string(1, mc.sign) + "i", "");
		}
		else if (mc.letter == 't')
		{
			channel->setTopicLocked(mc.sign == '+');
			broadcastMode(srv, client, channel, std::string(1, mc.sign) + "t", "");
		}
		else if (mc.letter == 'k')
		{
			if (mc.sign == '+')
			{
				if (mc.param.empty())
				{
					srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));
					continue;
				}
				channel->setKey(mc.param);
				broadcastMode(srv, client, channel, "+k", mc.param);
			}
			else
			{
				channel->setKey("");
				broadcastMode(srv, client, channel, "-k", "");
			}
		}
		else if (mc.letter == 'l')
		{
			if (mc.sign == '+')
			{
				if (mc.param.empty())
				{
					srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));
					continue;
				}
				int limit = std::atoi(mc.param.c_str());
				if (limit <= 0)
				{
					srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Invalid limit"));
					continue;
				}
				channel->setLimit(limit);
				broadcastMode(srv, client, channel, "+l", mc.param);
			}
			else
			{
				channel->setLimit(0);
				broadcastMode(srv, client, channel, "-l", "");
			}
		}
		else if (mc.letter == 'o')
		{
			if (mc.param.empty())
			{
				srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters"));
				continue;
			}
			Client* target = srv.getClientByNick(mc.param);
			if (!target)
			{
				srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, mc.param + " :No such nick"));
				continue;
			}
			if (!channel->isMember(*target))
			{
				srv.sendMsg(client, reply(srv, client, ERR_USERNOTINCHANNEL, mc.param + " " + channel->getName() + " :They aren't on that channel"));
				continue;
			}
			channel->setOperator(*target, mc.sign == '+');
			broadcastMode(srv, client, channel, std::string(1, mc.sign) + "o", mc.param);
		}
	}
}
