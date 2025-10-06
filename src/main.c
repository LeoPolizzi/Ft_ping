/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 17:02:15 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/30 16:29:33 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

struct pingdata	data;
struct timeval last_sent = {0, 0}, start_time = {0, 0}, end_time = {0, 0};
volatile bool	stop = false;
char *prog_name = NULL;
bool is_root = false;

void	sigint_handler(int signal MAYBE_UNUSED)
{
	stop = true;
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
	fprintf(stderr, "Usage: %s [OPTION...] HOST ...\n", prog_name);
	fprintf(stderr, HELP_MESSAGE);
}

void ending_stats()
{
	//   struct timeval diff;
	//   if (data.packinfo.nb_send > 1)
	// timersub(&end_time, &start_time, &diff);
	//   else
	//       diff.tv_sec = diff.tv_usec = 0;
    fflush(stdout);
    fprintf(stdout, "--- %s ping statistics ---\n", data.sockinfo.hostname);
    fprintf(stdout, "%d packets transmitted, %d received, ", data.packinfo.nb_send, data.packinfo.nb_ok);
    if (data.packinfo.nb_dup > 0)
        fprintf(stdout, "+%d duplicates, ", data.packinfo.nb_dup);
    if (data.packinfo.nb_send > 0)
    {
        if (data.packinfo.nb_ok > data.packinfo.nb_send)
            fprintf(stdout, "-- somebody is printing forged packets!\n");
		else
			fprintf(stdout, "%.0f%% packet loss\n", ((data.packinfo.nb_send - data.packinfo.nb_ok) / (double)data.packinfo.nb_send) * 100.0);
            // fprintf(stdout, "%.0f%% packet loss, time %.3fms\n", ((data.packinfo.nb_send - data.packinfo.nb_ok) / (double)data.packinfo.nb_send) * 100.0, (double)(diff.tv_sec * 1000 + diff.tv_usec / 1000));
    }
    if (data.packinfo.nb_ok > 0)
        fprintf(stdout, "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
            data.packinfo.min->tv_sec * 1e3 + data.packinfo.min->tv_usec / 1e3,
            data.packinfo.avg.tv_sec * 1e3 + data.packinfo.avg.tv_usec / 1e3,
            data.packinfo.max->tv_sec * 1e3 + data.packinfo.max->tv_usec / 1e3,
            data.packinfo.mdev.tv_sec * 1e3 + data.packinfo.mdev.tv_usec / 1e3);
}

int		main(int ac, char **av)
{
	prog_name = av[0];
	is_root = (geteuid() == 0);
	if (ac < 2)
		return (help_message(av[0]), EXIT_FAILURE);
	memset(&data, 0, sizeof(data));
	parse_opts(ac, av);
	if (!is_root)
	{
		fprintf(stderr, "%s: Lacking privilege for icmp socket.\n", av[0]);
		stop_ping(EXIT_FAILURE);
	}
	if (!init_socket(av[0]))
		stop_ping(EXIT_FAILURE);
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
			fprintf(stderr, "%s: missing host operand\nTry \'%s --help\' or \'%s --usage\' for more information.\n", av[0], av[0], av[0]);
			stop_ping(EXIT_FAILURE);
		}
	}
	if (data.opts.size == 0)
		data.opts.size = PING_DEFAULT_DATA_LEN;
	ping_loop();
	stop_ping(EXIT_SUCCESS);
}
