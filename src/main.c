/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 17:02:15 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/28 17:25:38 by lpolizzi         ###   ########.fr       */
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

void ending_stats()
{
	struct timeval	end_time;
	struct timeval	diff;

	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &diff);
	fflush (stdout);
	fprintf(stdout, "\n--- %s ping statistics ---\n", data.sockinfo.hostname);
	fprintf(stdout, "%d packets transmitted, %d received, ", data.packinfo.nb_send, data.packinfo.nb_ok);
	if (data.packinfo.nb_dup > 0)
	  fprintf(stdout, "+%d duplicates, ", data.packinfo.nb_dup);
	if (data.packinfo.nb_send > 0)
	{
		if (data.packinfo.nb_ok > data.packinfo.nb_send)
			fprintf(stdout, "-- somebody is printing forged packets!\n");
		else
			fprintf(stdout, "%.0f%% packet loss, time %zums\n", ((data.packinfo.nb_send - data.packinfo.nb_ok) / (double)data.packinfo.nb_send) * 100.0, (size_t)(diff.tv_sec * 1e3 + diff.tv_usec / 1e3));
	}
	if (data.packinfo.nb_ok > 0)
		fprintf(stdout, "rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms", data.packinfo.min->tv_sec * 1e3 + data.packinfo.min->tv_usec / 1e3, data.packinfo.avg.tv_sec * 1e3 + data.packinfo.avg.tv_usec / 1e3, data.packinfo.max->tv_sec * 1e3 + data.packinfo.max->tv_usec / 1e3, (data.packinfo.mdev.tv_sec * 1e3 + data.packinfo.mdev.tv_usec / 1e3));
	if (data.opts.preload > 0)
		fprintf(stdout, ", pipe %d", data.opts.preload);
	fprintf(stdout, "\n");
}

int		main(int ac, char **av)
{
	prog_name = av[0];
	if (ac < 2)
		return (help_message(av[0]), EXIT_FAILURE);
	memset(&data, 0, sizeof(data));
	parse_opts(ac, av);
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
			fprintf(stderr, "Usage:\n\t%s [options] <host>\n", av[0]);
			stop_ping(EXIT_FAILURE);
		}
	}
	if (data.opts.size == 0)
		data.opts.size = PING_DEFAULT_DATA_LEN;
	ping_loop();
	stop_ping(EXIT_SUCCESS);
}
