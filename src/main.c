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

struct timeval last_sent = {0, 0}, start_time = {0, 0};

volatile bool	stop = false;

char *prog_name = NULL;

void	sigint_handler(int signal MAYBE_UNUSED)
{
	stop = true;
}

struct rtt_node *create_node(struct timeval rtt)
{
	struct rtt_node *new_node = calloc(1, sizeof(struct rtt_node));
	if (!new_node)
		return (NULL);
	new_node->rtt = rtt;
	new_node->next = NULL;
	return (new_node);
}

void	add_rtt(struct timeval rtt)
{
	struct rtt_node *new_node = create_node(rtt);
	if (!new_node)
		return ;
	if (!data.packinfo.rtt_list)
	{
		data.packinfo.rtt_list = new_node;
		data.packinfo.rtt_last = new_node;
	}
	else
	{
		data.packinfo.rtt_last->next = new_node;
		data.packinfo.rtt_last = new_node;
	}
	if (rtt.tv_sec > data.packinfo.max->tv_sec || (rtt.tv_sec == data.packinfo.max->tv_sec && rtt.tv_usec > data.packinfo.max->tv_usec))
	{
		if (!data.packinfo.max)
			data.packinfo.max = calloc(1, sizeof(struct timeval));
		*data.packinfo.max = rtt;
	}
	else if (rtt.tv_sec < data.packinfo.min->tv_sec || (rtt.tv_sec == data.packinfo.min->tv_sec && rtt.tv_usec < data.packinfo.min->tv_usec))
	{
		if (!data.packinfo.min)
			data.packinfo.min = calloc(1, sizeof(struct timeval));
		*data.packinfo.min = rtt;
	}
	data.packinfo.avg.tv_sec = (data.packinfo.avg.tv_sec * (data.packinfo.nb_ok - 1) + rtt.tv_sec) / data.packinfo.nb_ok;
	data.packinfo.avg.tv_usec = (data.packinfo.avg.tv_usec * (data.packinfo.nb_ok - 1) + rtt.tv_usec) / data.packinfo.nb_ok;
	data.packinfo.mdev.tv_sec = (data.packinfo.mdev.tv_sec * (data.packinfo.nb_ok - 1) + (rtt.tv_sec - data.packinfo.avg.tv_sec) * (rtt.tv_sec - data.packinfo.avg.tv_sec)) / data.packinfo.nb_ok;
	data.packinfo.mdev.tv_usec = (data.packinfo.mdev.tv_usec * (data.packinfo.nb_ok - 1) + (rtt.tv_usec - data.packinfo.avg.tv_usec) * (rtt.tv_usec - data.packinfo.avg.tv_usec)) / data.packinfo.nb_ok;
}

void	free_list(struct rtt_node **head)
{
	struct rtt_node *current = *head;
	struct rtt_node *next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head = NULL;
}

void	stop_ping(int exit_code)
{
	free(data.sockinfo.hostname);
	if (data.sockinfo.sockfd > 0)
		close(data.sockinfo.sockfd);
	free_list(&data.packinfo.rtt_list);
	free(data.packinfo.min);
	free(data.packinfo.max);
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
	hints.ai_family = AF_INET;
	if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
	{
		fprintf(stderr, "%s: %s: No address associated with hostname\n", prog_name, hostname);
		return (false);
	}
	data.sockinfo.addr = *(struct sockaddr_in *)res->ai_addr;
	data.sockinfo.addr_len = res->ai_addrlen;
	data.sockinfo.hostname = strdup(hostname);
	if (inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, data.sockinfo.ip_str, sizeof(data.sockinfo.ip_str)) == NULL)
	{
		fprintf(stderr, "%s: inet_ntop: %s\n", prog_name, strerror(errno));
		freeaddrinfo(res);
		return (false);
	}
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
    if (data.opts.timeout != 0)
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
	if (data.opts.opt_mask & OPT_RROUTE)
	{
		unsigned char rr[40];
		rr[0] = 7;
		rr[1] = 39;
		rr[2] = 4;
		memset(rr+3, 0, 36);
		if (setsockopt(data.sockinfo.sockfd, IPPROTO_IP, IP_OPTIONS, rr, sizeof(rr)) < 0)
		{
			fprintf(stderr, "%s: setsockopt IP_OPTIONS: %s\n", prog_name, strerror(errno));
			return (false);
		}
	}
    return (true);
}

bool	send_ping()
{
	static int	sequence = 0;
	struct timeval	current;
	struct timeval	diff;

	if (data.opts.count > 0 && data.packinfo.nb_send >= data.opts.count)
	{
		stop = true;
		return (true);
	}
	if (!build_packet(sequence, &data.packinfo.packet))
		return (false);
	if (sendto(data.sockinfo.sockfd, &data.packinfo.packet, sizeof(data.packinfo.packet->hdr) + data.opts.size, 0, (struct sockaddr *)&data.sockinfo.addr, sizeof(data.sockinfo.addr)) <= 0)
	{
		fprintf(stderr, "%s: sendto: %s\n", prog_name, strerror(errno));
		free(data.packinfo.packet);
		return (false);
	}
	gettimeofday(&current, NULL);
	timersub(&current, &last_sent, &diff);
	last_sent = current;
	add_rtt(diff);
	if (!(data.opts.opt_mask & OPT_QUIET))
		fprintf(stdout, "%ld bytes from %s: icmp_seq=%d, time=%.2f ms\n", sizeof(data.packinfo.packet->hdr) + data.opts.size, data.sockinfo.ip_str, sequence, (diff.tv_sec * 1e3) + (diff.tv_usec / 1e3));
	free(data.packinfo.packet);
	data.packinfo.nb_send++;
	sequence++;
	return (true);
}

void ping_loop(void)
{
    signal(SIGINT, sigint_handler);
    gettimeofday(&start_time, NULL);
    last_sent = start_time;
    fprintf(stdout, "PING %s (%s) %d(%ld) bytes of data.\n", data.sockinfo.hostname,data.sockinfo.ip_str, (int)(sizeof(data.packinfo.packet->hdr) + data.opts.size), (long)(sizeof(data.packinfo.packet->hdr) + data.opts.size + 28));
    while (!stop)
    {
        if (!send_ping())
            break;
        long elapsed_us = data.packinfo.rtt_last->rtt.tv_sec * 1000000L + data.packinfo.rtt_last->rtt.tv_usec;
        long wait_us = SECOND_IN_USEC - elapsed_us;
        if (wait_us < 0)
            wait_us = 0;
        struct timeval timeout;
        timeout.tv_sec  = wait_us / 1000000L;
        timeout.tv_usec = wait_us % 1000000L;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(data.sockinfo.sockfd, &readfds);
        int ret = select(data.sockinfo.sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0)
		{
			if (errno != EINTR)
			{
				fprintf(stderr, "%s: select: %s\n", prog_name, strerror(errno));
				stop_ping(EXIT_FAILURE);
			}
			continue;
        }
        if (ret == 0)
            continue;
        if (FD_ISSET(data.sockinfo.sockfd, &readfds))
            if (!receive_ping())
                break;
    }
    if (!stop)
        sleep(data.opts.linger);
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
