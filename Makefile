# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/25 13:10:54 by lpolizzi          #+#    #+#              #
#    Updated: 2025/09/25 13:27:04 by lpolizzi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = cc

CFLAGS = -Wall -Wextra -Werror -O3 -funroll-loops -march=native -I./inc
LDFLAGS = -lm

NAME = ft_ping

SRCS =	src/main.c \
		src/parse_opts.c \
		src/ICMP_packet_builder.c \
		src/resolve_host.c \
		src/ping_loop.c \
		src/rtt_struct_utils.c \
		src/init_socket.c \
		src/lib_getopt/lib_getopt.c \

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Compiling $(NAME)...$(NC)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)
	@echo "$(GREEN)$(NAME) compiled successfully!$(NC)"

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

up: vclean
	@echo "$(YELLOW)Setting up Vagrant environment...$(NC)"
	@vagrant up
	@echo "$(GREEN)Vagrant environment is up and running!$(NC)"

clean:
	@echo "$(YELLOW)Cleaning object files...$(NC)"
	@rm -f $(OBJS)
	@echo "$(GREEN)Object files cleaned!$(NC)"

fclean: clean
	@echo "$(YELLOW)Removing $(NAME)...$(NC)"
	@rm -f $(NAME)
	@echo "$(GREEN)$(NAME) removed!$(NC)"

vclean:
	@echo "$(YELLOW)Cleaning Vagrant artifacts...$(NC)"
	@vagrant halt
	@rm -rf .vagrant
	@VBoxManage controlvm "debian7-vm" poweroff 2>/dev/null || true
	@VBoxManage unregistervm "debian7-vm" --delete 2>/dev/null || true
	@echo "$(GREEN)Vagrant artifacts cleaned!$(NC)"

re: fclean
	@$(MAKE) all

.PHONY: all clean fclean re
