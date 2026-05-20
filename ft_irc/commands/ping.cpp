#include "commands.hpp"

void cmd_ping(Server& srv, Client& client, const std::vector<std::string>& params)
{
	std::string token = params.empty() ? srv.getServerName() : params[0];
	srv.sendMsg(client, ":" + srv.getServerName() + " PONG " + srv.getServerName() + " :" + token + "\r\n");
}
