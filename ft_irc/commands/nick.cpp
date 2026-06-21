#include "commands.hpp"
#include <cctype>

//nova helper para ajudar com nicks, aparentemente o check antigo nao estava correto
static bool isSpecial(char c)
{
	return c == '_' || c == '[' || c == ']' || c == '\\' || c == '`'
	    || c == '^' || c == '{' || c == '|' || c == '}';
}

static bool isValidNick(const std::string& nick)
{
	if (nick.empty() || nick.size() > 30)
		return false;
	if (!std::isalpha(nick[0]) && !isSpecial(nick[0]))
		return false;
	for (size_t i = 1; i < nick.size(); ++i)
	{
		if (!std::isalnum(nick[i]) && nick[i] != '-' && !isSpecial(nick[i]))
			return false;
	}
	return true;
}

void cmd_nick(Server& srv, Client& client, const std::vector<std::string>& params)
{
	std::string newNick;
	Client*     existing;

	if (params.empty() || params[0].empty())
		return srv.sendMsg(client, reply(srv, client, ERR_NONICKNAMEGIVEN, ":No nickname given"));

	newNick = params[0];

	if (!isValidNick(newNick))
		return srv.sendMsg(client, reply(srv, client, ERR_ERRONEUSNICK, newNick + " :Erroneous nickname"));

	existing = srv.getClientByNick(newNick);
	if (existing && existing != &client)
		return srv.sendMsg(client, reply(srv, client, ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use"));
	//mundanca de nick para todos os canais, ficou um pouco confuso
	if (client.isRegistered())
	{
		std::string              nickMsg = ":" + client.getPrefix() + " NICK :" + newNick + "\r\n";
		std::set<Client*>        notified;

		srv.sendMsg(client, nickMsg);
		notified.insert(&client);

		const std::set<std::string>& channels = client.getChannelList();
		for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
		{
			Channel* channel = srv.getChannel(*it);
			if (!channel)
				continue;
			const std::map<Client*, bool>& members = channel->getMembers();
			for (std::map<Client*, bool>::const_iterator m = members.begin(); m != members.end(); ++m)
			{
				if (notified.find(m->first) == notified.end())
				{
					srv.sendMsg(*m->first, nickMsg);
					notified.insert(m->first);
				}
			}
		}
	}

	client.setNick(newNick);
	/* se senha for vazia e ja estiver setado o user, autentica o cliente */
	if (srv.getPassword().empty() && !client.getUser().empty())
		client.setAuthenticated(true);
	srv.checkRegistration(client);
}
