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
      src/commands/User.cpp

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
