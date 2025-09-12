all:
	cc -Wall -Wextra -Werror -g3 -fsanitize=address $(shell find . -name "*.c") -I./inc -o ft_ping
