#include "commands.hpp"
#include <sstream>

std::string reply(Server& srv, Client& client, const std::string& code, const std::string& rest)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();
	return ":" + srv.getServerName() + " " + code + " " + nick + " " + rest + "\r\n";
}

std::vector<std::string> split(const std::string& s, char delim)
{
	std::vector<std::string> result;
	std::stringstream        ss(s);
	std::string              token;
	while (std::getline(ss, token, delim))
		result.push_back(token);
	return result;
}
