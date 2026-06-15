#include "commands.hpp"

void	cmd_privmsg(Server& srv, Client& client, const std::vector<std::string>& params)
{
	/* verificacoes */ /* "#geral" "OLa" */ /* ola */
	if (params.empty())
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters")));
	/* splitar o params[0] por ',' e fazer um loop para enviar as mensagens para cada user/channel */
	if (params.size() == 1) /* pode ser simplicado para 'Not enough parameters' se foi diferente de 2 */
	{
		if ((!params[0].empty() && params[0][0] == '#') 
			|| srv.getClientByNick(params[0]) != NULL) /* somente recipiente (canal ou usuario), sem mensagem */
			return (srv.sendMsg(client, reply(srv, client, ERR_NOTEXTTOSEND, "PRIVMSG :No text to send")));
		/* nao passou canal nem usuario */
		return (srv.sendMsg(client, reply(srv, client, ERR_NORECIPIENT, "PRIVMSG :No recipient given")));
	}

	if (params.size() >= 2 && !params[0].empty() && !params[1].empty())
	{
		if (params[0][0] == '#') /* mensagem no grupo */
		{
			Channel*	channel = srv.getChannel(params[0]);
			if (!channel)
				return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel")));
			if (!channel->isMember(client))
				return (srv.sendMsg(client, reply(srv, client, ERR_NOTONCHANNEL, params[0] + " :Not channel member")));
	
			const std::string privMsg = ":" + client.getPrefix() + " PRIVMSG " + channel->getName() + " :" + params[1] + "\r\n";
			channel->broadcast(srv, client, privMsg);
		}
		else /* mensagem privada */
		{
			Client*	recipient = srv.getClientByNick(params[0]);
	
			if (!recipient)
				return (srv.sendMsg(client, reply(srv, client, ERR_NOSUCHNICK, params[0] + " :No such nick")));
			const std::string privMsg = ":" + client.getPrefix() + " PRIVMSG " + recipient->getNick() + " :" + params[1] + "\r\n";
			srv.sendMsg(*recipient, privMsg);
		}
	}
	else
		return (srv.sendMsg(client, reply(srv, client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters")));
}
