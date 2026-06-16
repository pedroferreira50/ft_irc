#include "commands.hpp"

void	cmd_invite(Server& srv, Client& client, const std::vector<std::string>& params)
{
	if (params.size() < 2)
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters")));
	Channel*	channel = srv.getChannel(params[1]);
	if (!channel)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[1] + " :No such channel")));
	if (!channel->isMember(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[1] + " :Not channel member")));
	if (!channel->isOperator(client))
		return (srv.sendMsg(client, reply(srv, client, ERR_CHANOPRIVSNEEDED, params[0] + " :Not channel operator")));
	
	Client* invited = srv.getClientByNick(params[0]);
	if (!invited)
		return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, params[0] + " :No such nick")));
	if (channel->isMember(*invited))
		return (srv.sendMsg(client, reply(srv, client, ERR_USERONCHANNEL, params[0] + " :User already member channel")));

	channel->addInvited(invited->getNick());
	srv.sendMsg(client, reply(srv, client, RPL_INVITING, invited->getNick() + " " + channel->getName()));
	std::string inviteMsg = ":" + client.getPrefix() + " INVITE " + invited->getNick() + " " + params[1] + "\r\n";
	srv.sendMsg(*invited, inviteMsg);
}
