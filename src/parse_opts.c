/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_opts.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 16:37:31 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/28 17:27:12 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static const struct option ping_longopts[] = {
	{"usage",         NO_ARG,       0,  L_OPT_USAGE},
	{"count",         REQ_ARG,		0, 'c'},
	{"debug",         NO_ARG,       0, 'd'},
	{"numeric",       NO_ARG,       0, 'n'},
	{"preload",       REQ_ARG,		0, 'l'},
	{"quiet",         NO_ARG,       0, 'q'},
	{"ignore-routing",NO_ARG,       0, 'r'},
	{"route",         NO_ARG,       0, 'R'},
	{"size",          REQ_ARG,		0, 's'},
    {"verbose",       NO_ARG,       0, 'v'},
	{"version",       NO_ARG,       0, 'V'},
    {"timeout",       REQ_ARG,		0, 'w'},
    {"linger",        REQ_ARG,		0, 'W'},
    {"help",          NO_ARG,       0, '?'},
    {0, 0, 0, 0}
};

size_t ping_convert_number (const char *optarg, size_t max_val, bool allow_zero)
{
	char *endptr;
 	unsigned long n;

 	n = strtoul (optarg, &endptr, 0);
 	if (*endptr)
 	{
		fprintf(stderr, "%s: error: invalid value (`%s' near `%s')", prog_name, optarg, endptr);
		stop_ping(EXIT_FAILURE);
 	}
 	if (n == 0 && !allow_zero)
	{
		fprintf(stderr, "%s: error: option value too small: %s", prog_name, optarg);
		stop_ping(EXIT_FAILURE);
	}
 	if (max_val && n > max_val)
	{
		fprintf(stderr, "%s: error: option value too big: %s", prog_name, optarg);
		stop_ping(EXIT_FAILURE);
	}
 	return (n);
}

void parse_opts(int ac, char **av)
{
    int c;

    while ((c = ft_getopt(ac, av, "dnqrRvV?c:l:s:w:W:", ping_longopts, NULL, false)) != -1)
    {
        switch (c)
        {
        case ('n'):
			data.opts.opt_mask |= OPT_NUMERIC;
            break;
        case ('q'):
			data.opts.opt_mask |= OPT_QUIET;
            break;
        case ('R'):
			data.opts.opt_mask |= OPT_RROUTE;
            break;
        case ('v'):
			data.opts.opt_mask |= OPT_VERBOSE;
            break;
		case ('d'):
			data.opts.sock_flags |= SO_DEBUG;
			break;
		case ('r'):
			data.opts.sock_flags |= SO_DONTROUTE;
			break;
		case ('c'):
			data.opts.count = ping_convert_number(g_optarg, 0, true);
			break;
		case ('l'):
			data.opts.preload = ping_convert_number(g_optarg, 1000, true);
			break;
		case ('s'):
			data.opts.size = ping_convert_number(g_optarg, PING_MAX_DATA_LEN, true);
			break;
        case ('w'):
            data.opts.timeout = ping_convert_number(g_optarg, INT_MAX, true);
            break;
        case ('W'):
            data.opts.linger = ping_convert_number(g_optarg, INT_MAX, true);
            break;
		case ('V'):
			printf("ft_ping version %s\n", VERSION);
			stop_ping(EXIT_SUCCESS);
			break;
		case (L_OPT_USAGE):
		   fprintf(stderr, "Usage:\n\t%s [options] <host>\n", av[0]);
		   stop_ping(EXIT_SUCCESS);
		   break;
        case ('?'):
            help_message(av[0]);
            stop_ping(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr, "Error: Unknown option '-%c'.\n", c);
            help_message(av[0]);
            stop_ping(EXIT_FAILURE);
            break;
        }
    }
}
