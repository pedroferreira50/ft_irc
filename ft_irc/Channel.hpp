#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>

class Client;
class Server;

class Channel
{
	private:
		std::string						_name;			/* "#GeRaL" digitado pelo usuario */
		std::string						_nameLower;		/* "#geral" padronizado */
		std::string						_topic;			/* topico atual */
		std::string						_key;			/* senha (+k), vazia se nao tiver */
		int								_limit;			/* limite de membros (+l), 0 = sem limite */
		bool							_inviteOnly;	/* modo +i */
		bool							_topicLocked;	/* modo +t (so operador muda topic) */
		std::map<Client*, bool>			_members;		/* Client* = true se for operador */
		std::vector<std::string>		_invited;		/* nicks convidados via INVITE */

	public:
		Channel(const std::string& name);
		~Channel();

		const std::string&				getName() const;
		const std::string&				getNameLower() const;
		const std::string&				getTopic() const;
		const std::string&				getKey() const;
		int								getLimit() const;
		const std::map<Client*, bool>&	getMembers() const;
		int								getMemberCount() const;
		std::string						getMemberList() const;
		std::string						getModeString() const;

		void							setTopic(const std::string& topic);
		void							setKey(const std::string& key);
		void							setLimit(int limit);
		void							setInviteOnly(bool value);
		void							setTopicLocked(bool value);
		void							setOperator(Client& client, bool value);

		bool							isInviteOnly() const;
		bool							isTopicLocked() const;
		void							addMember(Client& client, bool isOperator);
		void							removeMember(Client& client);
		bool							isMember(Client& client) const;
		bool							isOperator(Client& client) const;
		bool							isEmpty() const;
		void							addInvited(const std::string& nick);
		void							removeInvited(const std::string& nick);
		bool							isInvited(const std::string& nick) const;

		void							broadcast(Server& server, Client& sender, const std::string& msg);
		void							broadcastAll(Server& server, const std::string& msg);

		static std::string				toLower(const std::string& s);
		static bool						isValidName(const std::string& name);


};

#endif /* CHANNEL_HPP */