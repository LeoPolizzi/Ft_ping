/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 17:02:15 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/08 17:35:21 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

struct pingdata	data;

volatile bool	stop = false;

char *prog_name = NULL;

void	sigint_handler(int signal __attribute__((unused)))
{
	stop = true;
}

void	stop_ping(int exit_code)
{
	free(data.sockinfo.hostname);
	if (data.sockinfo.sockfd > 0)
		close(data.sockinfo.sockfd);
	exit(exit_code);
}

void	help_message(char *prog_name)
{
	fprintf(stderr, "Usage:\n\t%s [options] <host>\n\n", prog_name);
	fprintf(stderr, "Options:\n" USAGE_MESSAGE);
}

bool	resolve_hostname(char *prog_name, char *hostname)
{
	struct addrinfo	hints;
	struct addrinfo	*res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
	{
		fprintf(stderr, "%s: %s: No address associated with hostname\n", prog_name, hostname);
		return (false);
	}
	data.sockinfo.addr = *(struct sockaddr_in *)res->ai_addr;
	data.sockinfo.hostname = strdup(hostname);
	freeaddrinfo(res);
	return (true);
}

bool init_socket(char *prog_name)
{
    int one = 1;

    data.sockinfo.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (data.sockinfo.sockfd < 0)
    {
        if (errno == EPERM || errno == EACCES)
        {
            errno = 0;
            data.sockinfo.sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
            if (data.sockinfo.sockfd < 0)
            {
                fprintf(stderr, "%s: Lacking privilege for icmp socket.\n", prog_name);
                return (false);
            }
        }
        else
        {
            fprintf(stderr, "%s: %s\n", prog_name, strerror(errno));
            return (false);
        }
    }
    if (data.opts.sock_flags != 0)
        if (setsockopt(data.sockinfo.sockfd, SOL_SOCKET, data.opts.sock_flags, &one, sizeof(one)) < 0)
        {
            fprintf(stderr, "%s: setsockopt SO_*: %s\n", prog_name, strerror(errno));
            return (false);
        }
    if (data.opts.timeout > 0)
    {
        struct timeval timeout;
        timeout.tv_sec = data.opts.timeout;
        timeout.tv_usec = (data.opts.timeout - timeout.tv_sec) * 1e6;
        if (setsockopt(data.sockinfo.sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            fprintf(stderr, "%s: setsockopt SO_RCVTIMEO: %s\n", prog_name, strerror(errno));
            return (false);
        }
    }
    return (true);
}

void	ping_loop()
{
	signal(SIGINT, sigint_handler);
	while (!stop)
	{
		if (!send_ping())
			break ;
		if (!receive_ping())
			break ;
	}
	finish_stats();
}

int		main(int ac, char **av)
{
	prog_name = av[0];
	if (ac < 2)
		return (help_message(av[0]), EXIT_FAILURE);
	memset(&data, 0, sizeof(data));
	parse_opts(ac, av);
	for (int i = 1; i < ac; i++)
	{
		if (av[i][0] != '-')
		{
			if (!resolve_hostname(av[0], av[i]))
				stop_ping(EXIT_FAILURE);
			break ;
		}
		if (i == ac - 1)
		{
			fprintf(stderr, "Usage:\n\t%s [options] <host>\n", av[0]);
			stop_ping(EXIT_FAILURE);
		}
	}
	if (!init_socket(av[0]))
		stop_ping(EXIT_FAILURE);
	ping_loop();
	stop_ping(EXIT_SUCCESS);
}
