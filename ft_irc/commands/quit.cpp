#include "commands.hpp"

void    cmd_quit(Server& srv, Client& client, const std::vector<std::string>& params)
{
	std::string quitMsg = "";

	if (params.size() >= 1)
		quitMsg = ":" + client.getPrefix() + " QUIT :" + params[0] + "\r\n";
	else
		quitMsg = ":" + client.getPrefix() + " QUIT \r\n";
	std::set<std::string> channels = client.getChannelList();
	for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel* channel = srv.getChannel(*it);
		if (!channel)
			continue ;
		/* remove e deleta canal se for o ultimo membro */
		channel->broadcast(srv, client, quitMsg);
		channel->removeMember(client);
		client.leaveChannel(*it);
		srv.removeEmptyChannel(*it);
	}
	std::string reason = params.size() >= 1 ? params[0] : "Client Quit";
	srv.sendMsg(client, "ERROR :Quit: " + reason + "\r\n");
	client.setDC();
}
