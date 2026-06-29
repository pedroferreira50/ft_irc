*This project has been created as part of the 42 curriculum by login1, login2, login3.*

# ft_irc

## Description

`ft_irc` is an implementation of an IRC (Internet Relay Chat) server written from scratch in C++98, without using any external networking or event libraries. The goal of the project is to understand and apply low-level network programming concepts: TCP sockets, non-blocking I/O, and multiplexing multiple client connections through a single event loop (`poll()`).

The server implements the core of the IRC protocol well enough to be used with a real IRC client (e.g. irssi, WeeChat, HexChat). A connecting client must authenticate with a server password and register a nickname and username before it can join channels, exchange messages, and manage channel state.

Supported commands:

| Command   | Purpose                                              |
|-----------|-------------------------------------------------------|
| `PASS`    | Authenticate with the server password                |
| `NICK`    | Set/change nickname                                   |
| `USER`    | Set username/realname and complete registration       |
| `PING`    | Keep-alive / liveness check                           |
| `JOIN`    | Join one or more channels (supports channel keys)     |
| `PART`    | Leave one or more channels                            |
| `PRIVMSG` | Send a message to a user or a channel                 |
| `QUIT`    | Disconnect from the server                            |
| `MODE`    | Get/set channel modes: `+i`, `+t`, `+k`, `+l`, `+o`    |
| `INVITE`  | Invite a user to an invite-only channel               |
| `KICK`    | Remove a user from a channel                          |
| `TOPIC`   | View or set a channel's topic                         |

Channel modes implemented:
- `+i` invite-only
- `+t` topic settable by operators only
- `+k` channel key (password)
- `+l` member limit
- `+o` operator privilege

## Instructions

### Requirements

- A C++98-compatible compiler (`c++`/`g++`/`clang++`)
- `make`
- A POSIX-compliant environment (Linux/macOS, or WSL on Windows)

### Compilation

```sh
make        # builds the ircserv binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

This produces an `ircserv` executable in the project root.

### Running the server

```sh
./ircserv <port> <password>
```

- `<port>`: TCP port the server listens on (1–65535)
- `<password>`: password required by clients via the `PASS` command

Example:

```sh
./ircserv 6667 mypassword
```

### Connecting to the server

Any standard IRC client can be used, for example with `nc`:

```sh
nc 127.0.0.1 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :hello everyone
```

Or with a real IRC client (e.g. `irssi`):

```sh
irssi -c 127.0.0.1 -p 6667 -w mypassword -n alice
```

## Resources

### IRC protocol & networking references

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [RFC 2813 — Internet Relay Chat: Server Protocol](https://www.rfc-editor.org/rfc/rfc2813)
- `man poll`, `man socket`, `man bind`, `man listen`, `man accept`, `man fcntl`

### AI usage

AI (Claude) was used as a supporting tool during development, specifically for:

- **Debugging**: investigating edge cases in command parsing, client registration flow, and channel mode handling, several of which are flagged in the code with `@QUESTION` / `@TODO` comments for follow-up.
- **Code review and explanation**: clarifying IRC protocol details (numeric reply codes, message framing) and reviewing non-blocking socket/`poll()` patterns against intended behavior.
- **Documentation**: drafting this `README.md` based on a review of the existing source code.

All core design decisions and the implementation itself were written and validated by the project authors; AI was not used to autonomously generate the project from scratch.
