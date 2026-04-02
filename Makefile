NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = src/main.cpp \
      src/Server.cpp \
      src/Client.cpp \
      src/Channel.cpp \
      src/Message.cpp \
      src/commands/Pass.cpp \
      src/commands/Nick.cpp \
      src/commands/User.cpp \
      src/commands/Quit.cpp \
      src/commands/Join.cpp \
      src/commands/Part.cpp \
      src/commands/Privmsg.cpp \
      src/commands/Notice.cpp \
      src/commands/Kick.cpp \
      src/commands/Invite.cpp \
      src/commands/Topic.cpp \
      src/commands/Mode.cpp \
      src/commands/Ping.cpp \
      src/commands/Who.cpp

INC = -Iinc

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
