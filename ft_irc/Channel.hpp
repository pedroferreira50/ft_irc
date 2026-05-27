#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>

#include "Client.hpp"

class Channel
{
	private:
		std::string					_name;			/* "#geral" */
		std::string					_topic;			/* topico atual */
		std::string					_key;			/* senha (+k), vazia se nao tiver */
		int							_limit;			/* limite de membros (+l), 0 = sem limite */
		bool						_inviteOnly;	/* modo +i */
		bool						_topicLocked;	/* modo +t (so operador muda topic) */
		std::map<Client*, bool>		_members;		/* Client* = true se for operador */
		std::vector<std::string>	_invited;		/* nicks convidados via INVITE */

	public:
		Channel();
		~Channel();
};

#endif /* CHANNEL_HPP */