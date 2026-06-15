#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "../Server.hpp"
# include "../Client.hpp"
# include "../Reply.hpp"
# include "../Channel.hpp"
# include <vector>
# include <string>

// Builds :servername CODE clientnick rest\r\n
inline std::string reply(Server& srv, Client& client,
                         const std::string& code, const std::string& rest)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();
	return ":" + srv.getServerName() + " " + code + " " + nick + " " + rest + "\r\n";
}

void cmd_pass(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_nick(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_user(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_ping(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_join(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_part(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_privmsg(Server& srv, Client& client, const std::vector<std::string>& params);
void cmd_quit(Server& srv, Client& client, const std::vector<std::string>& params);

#endif
