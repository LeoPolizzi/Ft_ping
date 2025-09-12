/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_opts.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 16:37:31 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/08 16:15:51 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static const struct option ping_longopts[] = {
    {"address",       NO_ARG,       0,  L_OPT_ADDRESS},
    {"mask",          NO_ARG,       0,  L_OPT_MASK},
    {"echo",          NO_ARG,       0,  L_OPT_ECHO},
	{"timestamp",     NO_ARG,       0,  L_OPT_TIMESTAMP},
	{"ip-timestamp",  NO_ARG,       0,  L_OPT_IP_TIMESTAMP},
	{"usage",         NO_ARG,       0,  L_OPT_USAGE},
	{"count",         REQ_ARG,		0, 'c'},
	{"debug",         NO_ARG,       0, 'd'},
	{"flood",         NO_ARG,       0, 'f'},
	{"interval",      REQ_ARG,		0, 'i'},
	{"preload",       REQ_ARG,		0, 'l'},
	{"numeric",       NO_ARG,       0, 'n'},
	{"pattern",       REQ_ARG,		0, 'p'},
	{"quiet",         NO_ARG,       0, 'q'},
	{"ignore-routing",NO_ARG,       0, 'r'},
	{"route",         NO_ARG,       0, 'R'},
	{"size",          REQ_ARG,		0, 's'},
	{"ttl",           REQ_ARG,		0, 't'},
	{"tos",           REQ_ARG,		0, 'T'},
    {"verbose",       NO_ARG,       0, 'v'},
	{"version",       NO_ARG,       0, 'V'},
    {"timeout",       REQ_ARG,		0, 'w'},
    {"linger",        REQ_ARG,		0, 'W'},
    {"help",          NO_ARG,       0, '?'},
    {0, 0, 0, 0}
};

static bool	valid_pattern(char *pattern)
{
	if (!pattern || strlen(pattern) % 2 != 0)
		return (false);
	for (int i = 0; pattern[i]; i++)
		if (!(pattern[i] >= '0' && pattern[i] <= '9')
			&& !(pattern[i] >= 'a' && pattern[i] <= 'f')
			&& !(pattern[i] >= 'A' && pattern[i] <= 'F'))
			return (false);
	return (true);
}

long strtol_wrapper(const char *nptr, char **endptr, int base, long long min, long long max, const char *opt_name)
{
	errno = 0;
	if (!nptr || *nptr == '\0')
	{
		fprintf(stderr, "Error: --%s option requires a numeric value.\n", opt_name);
		stop_ping(EXIT_FAILURE);
	}
	long value = strtol(nptr, endptr, base);
	if (errno == ERANGE || value < min || value > max || (endptr && *endptr && **endptr != '\0'))
	{
		fprintf(stderr, "Error: --%s option requires a value between %lld and %lld.\n", opt_name, min, max);
		stop_ping(EXIT_FAILURE);
	}
	return (value);
}

double strtod_wrapper(const char *nptr, char **endptr, double min, double max, const char *opt_name)
{
	errno = 0;
	if (!nptr || *nptr == '\0')
	{
		fprintf(stderr, "Error: --%s option requires a numeric value.\n", opt_name);
		stop_ping(EXIT_FAILURE);
	}
	double value = strtod(nptr, endptr);
	if (errno == ERANGE || value < min || value > max || (endptr && *endptr && **endptr != '\0'))
	{
		fprintf(stderr, "Error: --%s option requires a value between %.2f and %.2f.\n", opt_name, min, max);
		stop_ping(EXIT_FAILURE);
	}
	return (value);
}

void parse_opts(int ac, char **av)
{
    int c;
    char *endptr;

    while ((c = ft_getopt(ac, av, "dfnqrRvV?c:i:l:p:s:t:T:w:W:", ping_longopts, NULL, false)) != -1)
    {
        switch (c)
        {
        case (L_OPT_ADDRESS):
            if (geteuid() != 0)
            {
                fprintf(stderr, "Error: --address option requires root privileges.\n");
                stop_ping(EXIT_FAILURE);
            }
            data.opts.address = true;
            break;
        case (L_OPT_MASK):
            if (geteuid() != 0)
            {
                fprintf(stderr, "Error: --mask option requires root privileges.\n");
                stop_ping(EXIT_FAILURE);
            }
            data.opts.mask = true;
            break;
        case (L_OPT_ECHO):
            data.opts.echo = true;
            break;
        case (L_OPT_USAGE):
            fprintf(stderr, "Usage:\n\t%s [options] <host>\n", av[0]);
            stop_ping(EXIT_SUCCESS);
            break;
        case ('c'):
            data.opts.count = strtol_wrapper(g_optarg, &endptr, 10, 1, 9223372036854775807, "count");
            break;
        case ('d'):
			data.opts.sock_flags |= SO_DEBUG;
            break;
        case ('f'):
            if (geteuid() != 0)
            {
                fprintf(stderr, "Error: --flood option requires root privileges.\n");
                stop_ping(EXIT_FAILURE);
            }
			data.opts.opt_mask |= OPT_FLOOD;
            break;
        case ('i'):
            data.opts.interval = strtod_wrapper(g_optarg, &endptr, 0.01, 3600.0, "interval");
			data.opts.opt_mask |= OPT_INTERVAL;
            break;
        case ('l'):
            data.opts.preload = strtol_wrapper(g_optarg, &endptr, 10, 0, 65535, "preload");
            break;
        case ('n'):
			data.opts.opt_mask |= OPT_NUMERIC;
            break;
        case ('p'):
            if (!valid_pattern(g_optarg))
            {
                fprintf(stderr, "Error: invalid pattern: Pattern must contain only hexadecimal values.\n");
                stop_ping(EXIT_FAILURE);
            }
            data.opts.pattern = strndup(g_optarg, 16);
            break;
        case ('q'):
			data.opts.opt_mask |= OPT_QUIET;
            break;
        case ('r'):
			data.opts.sock_flags |= SO_DONTROUTE;
            break;
        case ('R'):
			data.opts.opt_mask |= OPT_RROUTE;
            break;
        case ('s'):
            data.opts.size = strtol_wrapper(g_optarg, &endptr, 10, 0, MAX_PAYLOAD_SIZE, "size");
            break;
        case ('t'):
            data.opts.ttl = strtol_wrapper(g_optarg, &endptr, 10, 1, 255, "ttl");
            break;
        case ('T'):
            data.opts.tos = strtol_wrapper(g_optarg, &endptr, 10, 0, 255, "tos");
            break;
        case ('v'):
			data.opts.opt_mask |= OPT_VERBOSE;
            break;
        case ('V'):
            printf("ft_ping version %s\n", VERSION);
            stop_ping(EXIT_SUCCESS);
            break;
        case ('w'):
            data.opts.timeout = strtol_wrapper(g_optarg, &endptr, 10, 1, 9223372036854775807, "timeout");
            break;
        case ('W'):
            data.opts.linger = strtol_wrapper(g_optarg, &endptr, 10, 1, 9223372036854775807, "linger");
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
