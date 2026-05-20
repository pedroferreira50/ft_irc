#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

class Client
{
private:
	int         _fd;
	std::string _hostname;
	std::string _nick;
	std::string _user;
	std::string _realname;
	bool        _authenticated;
	bool        _registered;
	std::string _readBuf;
	std::string _writeBuf;

public:
	Client(int fd, const std::string& hostname);
	~Client();

	int                 getFd()           const;
	const std::string&  getHostname()     const;
	std::string         getPrefix()       const;
	const std::string&  getNick()         const;
	const std::string&  getUser()         const;
	const std::string&  getRealname()     const;
	bool                isAuthenticated() const;
	bool                isRegistered()    const;

	void setNick(const std::string& nick);
	void setUser(const std::string& user);
	void setRealname(const std::string& realname);
	void setAuthenticated(bool val);
	void setRegistered(bool val);

	std::string& getReadBuf();
	std::string& getWriteBuf();
};

#endif
