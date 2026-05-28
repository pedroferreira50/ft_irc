NAME     = ircserv
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS     = main.cpp \
           Server.cpp \
           Client.cpp \
           Parser.cpp \
           Channel.cpp \
           commands/pass.cpp \
           commands/nick.cpp \
           commands/user.cpp \
           commands/ping.cpp

OBJ_DIR  = obj
OBJS     = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
