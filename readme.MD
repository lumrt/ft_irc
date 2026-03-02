*This project has been created as part of the 42 curriculum.*

# ft_irc — IRC Server

An IRC server written in C++98, compatible with standard IRC clients (irssi, hexchat, WeeChat…).

## Description

ft_irc is a single-process, non-blocking IRC server using `poll()` for I/O multiplexing. It handles multiple simultaneous client connections over TCP/IP v4 and implements the core subset of the IRC protocol required by the 42 subject.

## How to Build

```bash
make        # compile
make re     # full recompile
make clean  # remove object files
make fclean # remove object files + binary
```

Requires a C++98-compatible compiler. Compiled with `-Wall -Wextra -Werror -std=c++98`.

## How to Run

```bash
./ircserv <port> <password>
```

- **port**: TCP port the server listens on (e.g. `6667`)
- **password**: connection password required by all clients

Example:

```bash
./ircserv 6667 mypass
```

## Testing

### With nc (netcat)

Basic registration:

```bash
nc -C 127.0.0.1 6667
PASS mypass
NICK mynick
USER myuser 0 * :My Real Name
```

You should receive the welcome replies (001–004).

Wrong password:

```bash
printf "PASS wrong\r\nNICK test\r\nUSER t 0 * :T\r\n" | nc -w 2 127.0.0.1 6667
# Expected: 464 :Password incorrect
```

Missing NICK parameter:

```bash
printf "PASS mypass\r\nNICK\r\n" | nc -w 2 127.0.0.1 6667
# Expected: 431 :No nickname given
```

Command before registration:

```bash
printf "PRIVMSG #test :hello\r\n" | nc -w 2 127.0.0.1 6667
# Expected: 451 :You have not registered
```

Partial data / buffering (ctrl+D test from subject):

```bash
nc -C 127.0.0.1 6667
# Type: PAS  then ctrl+D
# Type: S my then ctrl+D
# Type: pass then Enter
# The server must reconstruct "PASS mypass" from fragments
```

### With an IRC client (irssi)

```bash
irssi -c 127.0.0.1 -p 6667 -w mypass -n mynick
```

You should connect and see the welcome message.

## Project Architecture

```
inc/                    Headers
    Server.hpp          Server class (network + dispatch)
    Client.hpp          Client class (state per connection)
    Channel.hpp         Channel class (members, operators, topic)
    Message.hpp         IRC message struct + parser

src/                    Implementation
    main.cpp            Entry point, argument parsing
    Server.cpp          Socket init, poll loop, buffering, dispatch, send utilities
    Client.cpp          Client state (getters/setters, prefix)
    Channel.cpp         Channel logic (members, operators, topic)
    Message.cpp         IRC message parser (prefix, command, params, trailing)
    commands/
        Pass.cpp        PASS command — password authentication
        Nick.cpp        NICK command — set/change nickname
        User.cpp        USER command — set username and realname
```

## How It Works (overview for teammates)

### Network Layer (`Server`)

1. `initSocket()` — creates a non-blocking TCP socket, binds, listens
2. `run()` — main `poll()` loop: watches server fd + all client fds for POLLIN
3. `acceptClient()` — accepts new connection, sets non-blocking, adds to poll set
4. `receiveData(fd)` — reads into client buffer, extracts complete lines on `\n` (strips `\r`), hands each to `handleLine()`
5. `disconnectClient(fd)` — closes fd, removes from maps and channels

### Message Parsing (`Message` + `parseMessage`)

Parses standard IRC format: `[:prefix] COMMAND [param1 param2 ...] [:trailing with spaces]`

- Command is uppercased for case-insensitive matching
- Trailing parameter (after `:`) is stored as a single param containing spaces

### Command Dispatch (`handleLine`)

Simple if/else chain in `Server::handleLine()`. To add a new command:

1. Declare `void cmdXxx(int fd, const Message &msg);` in `Server.hpp`
2. Create `src/commands/Xxx.cpp` implementing it
3. Add the `if (msg.command == "XXX") return cmdXxx(fd, msg);` line in `handleLine()`
4. Add the `.cpp` to the Makefile `SRC` list

### Registration Flow

A client must complete three steps to register:

1. **PASS** — correct password → sets `passAccepted = true`
2. **NICK** — valid, unique nickname → sets nickname
3. **USER** — username + realname → sets username/realname

`tryRegister()` checks all three conditions. Once met, sends `001`–`004` welcome replies and marks the client as registered. Any non-registration command before completion returns `451 ERR_NOTREGISTERED`.

### Client State (`Client`)

| Field          | Purpose                              |
|----------------|--------------------------------------|
| `_fd`          | Socket file descriptor               |
| `_buffer`      | Incomplete data accumulator          |
| `_nickname`    | IRC nickname                         |
| `_username`    | Username from USER command           |
| `_realname`    | Real name from USER command          |
| `_passAccepted`| Password verified                    |
| `_registered`  | Full registration completed          |

### Channel (`Channel`)

| Field        | Purpose                                |
|--------------|----------------------------------------|
| `_name`      | Channel name (e.g. `#general`)         |
| `_members`   | Set of member fds                      |
| `_operators` | Set of operator fds (first joiner auto)|
| `_topic`     | Channel topic string                   |

### Error Replies Implemented

| Code | Name                 | Trigger                          |
|------|----------------------|----------------------------------|
| 431  | ERR_NONICKNAMEGIVEN  | NICK without parameter           |
| 432  | ERR_ERRONEUSNICKNAME | Invalid nickname characters      |
| 433  | ERR_NICKNAMEINUSE    | Nickname already taken           |
| 451  | ERR_NOTREGISTERED    | Command before registration      |
| 461  | ERR_NEEDMOREPARAMS   | Missing required parameters      |
| 462  | ERR_ALREADYREGISTRED | PASS/USER after registration     |
| 464  | ERR_PASSWDMISMATCH   | Wrong password                   |

## What Still Needs to Be Implemented

- [ ] **JOIN** / **PART** — join and leave channels
- [ ] **PRIVMSG** — private messages and channel messages
- [ ] **QUIT** — clean disconnection with quit message
- [ ] **KICK** — eject user from channel (operator)
- [ ] **INVITE** — invite user to channel (operator)
- [ ] **TOPIC** — view/change channel topic (operator if +t)
- [ ] **MODE** — channel modes: `i` (invite-only), `t` (topic lock), `k` (key), `o` (operator), `l` (user limit)
- [ ] **PING/PONG** — keepalive mechanism

## Resources

- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC 1459 — Original IRC Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [Modern IRC Docs](https://modern.ircdocs.horse/)
- [IRC Numerics Reference](https://www.alien.net.au/irc/irc2numerics.html)
