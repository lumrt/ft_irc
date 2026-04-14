*This project has been created as part of the 42 curriculum.*

# ft_irc — IRC Server

A single-process, non-blocking IRC server in C++98 using `poll()` for I/O multiplexing. Handles multiple simultaneous clients over TCP/IPv4.

## Build

```bash
make        # compile
make re     # full recompile
make clean  # remove .o files
make fclean # remove .o + binary
```

Flags: `-Wall -Wextra -Werror -std=c++98`

## Run

```bash
./ircserv <port> <password>
```

## Test with irssi (reference client)

```bash
irssi -c 127.0.0.1 -p 6667 -w mypass -n mynick
```

## Test with nc

```bash
nc -C 127.0.0.1 6667
PASS mypass
NICK test
USER test 0 * :Test User
JOIN #channel
PRIVMSG #channel :Hello
QUIT
```

## Project Structure

```
inc/
    Server.hpp     Server class (network, dispatch, signal handling)
    Client.hpp     Client state (fd, nick, user, buffer, registration)
    Channel.hpp    Channel (members, operators, modes i/t/k/l, invites, topic)
    Message.hpp    IRC message parser + splitList utility

src/
    main.cpp       Entry point, argument validation, try/catch
    Server.cpp     Socket, poll loop, buffering, dispatch, send, signals
    Client.cpp     Getters/setters, prefix generation
    Channel.cpp    Members, operators, modes, invite list
    Message.cpp    IRC parser (prefix, command, params, trailing)
    commands/
        Pass.cpp       PASS  — connection password
        Nick.cpp       NICK  — set/change nickname (broadcast to channels)
        User.cpp       USER  — set username + realname
        Quit.cpp       QUIT  — clean disconnection
        Join.cpp       JOIN  — join channels (with +i/+k/+l checks)
        Part.cpp       PART  — leave channels
        Privmsg.cpp    PRIVMSG — channel + private messages
        Notice.cpp     NOTICE  — like PRIVMSG, no error replies
        Kick.cpp       KICK  — eject user (operator only)
        Invite.cpp     INVITE — invite to +i channel (operator only)
        Topic.cpp      TOPIC — view/change topic (+t = operator only)
        Mode.cpp       MODE  — channel modes: i, t, k, o, l
        Ping.cpp       PING  — keepalive (responds PONG)
        Who.cpp        WHO   — channel member list
```

## Implemented Features

- [x] Authentication (PASS), nickname (NICK), username (USER)
- [x] Join/leave channels (JOIN, PART)
- [x] Private and channel messages (PRIVMSG, NOTICE)
- [x] Operator commands: KICK, INVITE, TOPIC
- [x] Channel modes: invite-only (+i), topic lock (+t), key (+k), operator (+o), user limit (+l)
- [x] Clean disconnection (QUIT) with broadcast to channels
- [x] PING/PONG keepalive
- [x] Signal handling (SIGINT, SIGTERM, SIGPIPE)
- [x] Partial data reconstruction (buffering on `\r\n`)
- [x] Non-blocking I/O, single poll() for all operations

## Resources

- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC 1459 — Original IRC Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [Modern IRC Docs](https://modern.ircdocs.horse/)
