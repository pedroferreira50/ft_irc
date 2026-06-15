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

	std::vector<std::string> targets = split(params[0], ',');

	for (size_t i = 0; i < targets.size(); ++i)
	{
		// preciso testar se este check e correto, para caso empty do tipo PRIVMSG #channel msg1, :hello (virgula faz segundo parameter empty)
		if (targets[i].empty())
			continue ;
		if (params.size() >= 2 && !targets[i].empty() && !params[1].empty())
		{
			if (targets[i][0] == '#') /* mensagem no grupo */
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
			else /* mensagem privada */
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
