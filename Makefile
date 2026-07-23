# Compiler and Flags
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Executable Name
NAME     = cgi_test

# Source and Object Files
SRCS     = main.cpp CGI.cpp
OBJS     = $(SRCS:.cpp=.o)

# Default Rule
all: $(NAME)

# Link Object Files to Create Executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compile .cpp files into .o Object Files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Object Files
clean:
	rm -f $(OBJS)

# Clean Object Files and Executable
fclean: clean
	rm -f $(NAME)

# Re-build everything
re: fclean all

.PHONY: all clean fclean re