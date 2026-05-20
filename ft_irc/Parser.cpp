#include "Parser.hpp"
#include <cctype>

Message Parser::parse(const std::string& raw)
{
	Message     msg;
	std::string line = raw;
	size_t      pos;
	size_t      end;

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	if (line.empty())
		return msg;

	pos = 0;

	if (line[pos] == ':')
	{
		end = line.find(' ', pos);
		if (end == std::string::npos)
			return msg;
		msg.prefix = line.substr(1, end - 1);
		pos = end + 1;
		while (pos < line.size() && line[pos] == ' ')
			++pos;
	}

	end = line.find(' ', pos);
	if (end == std::string::npos)
	{
		msg.command = line.substr(pos);
		for (size_t i = 0; i < msg.command.size(); ++i)
			msg.command[i] = std::toupper(msg.command[i]);
		return msg;
	}
	msg.command = line.substr(pos, end - pos);
	for (size_t i = 0; i < msg.command.size(); ++i)
		msg.command[i] = std::toupper(msg.command[i]);
	pos = end + 1;

	while (pos < line.size())
	{
		while (pos < line.size() && line[pos] == ' ')
			++pos;
		if (pos >= line.size())
			break;

		if (line[pos] == ':')
		{
			msg.params.push_back(line.substr(pos + 1));
			break;
		}

		end = line.find(' ', pos);
		if (end == std::string::npos)
		{
			msg.params.push_back(line.substr(pos));
			break;
		}
		msg.params.push_back(line.substr(pos, end - pos));
		pos = end + 1;
	}

	return msg;
}
