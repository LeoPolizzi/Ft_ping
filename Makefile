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

YELLOW = \033[1;33m
GREEN = \033[1;32m
RED = \033[1;31m
NC = \033[0m

CC = clang

CFLAGS = -Wall -Wextra -Werror -Wpedantic -I./inc

OPTIFLAGS = -O3 -ffast-math -march=native -mtune=native -funroll-loops \
			-fvectorize -fslp-vectorize -fstrict-aliasing \
			-fno-math-errno -fomit-frame-pointer -fmerge-all-constants \
			-freciprocal-math -funsafe-math-optimizations \
			-finline-functions -fno-trapping-math -falign-functions=32 \
			-falign-loops=32 -fno-stack-protector \
			-fdata-sections -ffunction-sections

LDFLAGS = -flto -Wl,--gc-sections -lm

NAME = ft_ping

SRCS =	src/main.c \
		src/parse_opts.c \
		src/ICMP_packet_builder.c \
		src/resolve_host.c \
		src/ping_loop.c \
		src/rtt_struct_utils.c \
		src/init_socket.c \
		src/ft_getopt/ft_getopt.c \

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo -e "$(YELLOW)Compiling $(NAME)...$(NC)"
	@$(CC) $(CFLAGS) $(OPTIFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)
	@echo -e "$(GREEN)$(NAME) compiled successfully!$(NC)"
	@if [ "$$(id -u)" -eq 0 ]; then \
		echo -e "$(YELLOW)Setting SUID bit on $(NAME)...$(NC)"; \
		make setuid; \
	else \
		echo -e "$(RED)Warning: You are not root. SUID bit not set. Run 'sudo make setuid' to set it.$(NC)"; \
	fi

setuid:
	@if [ "$$(id -u)" -ne 0 ]; then \
		echo -e "$(RED)Error: You must be root to set the SUID bit.$(NC)"; \
		exit 1; \
	fi
	@sudo chown root:root $(NAME)
	@sudo chmod +s $(NAME)
	@echo -e "$(GREEN)SUID bit set on $(NAME)!$(NC)"

%.o: %.c
	@$(CC) $(CFLAGS) $(OPTIFLAGS) -c $< -o $@

clean:
	@echo -e "$(YELLOW)Cleaning object files...$(NC)"
	@rm -f $(OBJS)
	@echo -e "$(GREEN)Object files cleaned!$(NC)"

fclean: clean
	@echo -e "$(YELLOW)Removing $(NAME)...$(NC)"
	@rm -f $(NAME)
	@echo -e "$(GREEN)$(NAME) removed!$(NC)"

re: fclean
	@$(MAKE) all

.PHONY: all clean fclean re
