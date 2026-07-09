#include "commands.hpp"

void	cmd_privmsg(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters")));
	if (params.size() == 1)
	{
		if ((!params[0].empty() && (params[0][0] == '#' || params[0][0] == '&')) 
			|| srv.getClientByNick(params[0]) != NULL)
			return (srv.sendMsg(client, reply(srv, client, ERR_NOTEXTTOSEND, ":No text to send")));
		return (srv.sendMsg(client, reply(srv, client, ERR_NORECIPIENT, ":No recipient given (PRIVMSG)")));
	}

	std::vector<std::string> targets = split(params[0], ',');

	for (size_t i = 0; i < targets.size(); ++i)
	{
		if (targets[i].empty())
			continue ;
		if (params.size() >= 2 && !targets[i].empty())
		{
			if (params[1].empty())
			{
				srv.sendMsg(client, reply(srv, client, ERR_NOTEXTTOSEND, ":No text to send"));
				continue ;
			}
			if (targets[i][0] == '#' || targets[i][0] == '&')
			{
				Channel*	channel = srv.getChannel(targets[i]);
				if (!channel)
				{
					srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, targets[i] + " :No such channel"));
					continue ;
				}
				if (!channel->isMember(client))
				{
					srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, targets[i] + " :Not channel member"));
					continue ;
				}	
		
				const std::string privMsg = ":" + client.getPrefix() + " PRIVMSG " + channel->getName() + " :" + params[1] + "\r\n";
				channel->broadcast(srv, client, privMsg);
			}
			else
			{
				Client*	recipient = srv.getClientByNick(targets[i]);
		
				if (!recipient)
				{
					srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, targets[i] + " :No such nick"));
					continue ;
				}
				const std::string privMsg = ":" + client.getPrefix() + " PRIVMSG " + recipient->getNick() + " :" + params[1] + "\r\n";
				srv.sendMsg(*recipient, privMsg);
			}
		}
		else
		{
			srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters"));
			continue ;
		}
	}
}
