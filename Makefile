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
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
