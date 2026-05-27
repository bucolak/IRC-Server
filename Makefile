NAME = ircserv

SRC = main.cpp srcs/Channel.cpp srcs/Server.cpp srcs/Client.cpp srcs/Server_authentication.cpp srcs/Server_chnl.cpp srcs/Server_messg.cpp srcs/Server_utils.cpp srcs/Server_executer.cpp

OBJ = $(SRC:.cpp=.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re