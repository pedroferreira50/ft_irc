#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <stdexcept>

int main(int argc, char **argv)
{
	int port;

	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Error: invalid port" << std::endl;
		return 1;
	}
	std::string password = argv[2];
	if (password.empty())
		return (std::cerr << "Error: invalid password" << std::endl, 1);

	try
	{
		Server server(port, argv[2]);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
