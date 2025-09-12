/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib_getopt.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 14:34:56 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/08 16:02:10 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib_getopt.h"

int		g_optind = 1,
		g_opterr = 1,
		g_optopt = 0;

char	*g_optarg = NULL;

void	ft_getopt_reset(void)
{
	g_optind = 1;
	g_optarg = NULL;
	g_optopt = 0;
}

static short	gettype(char opt, const char *opt_string)
{
	for (int i = 0; opt_string[i]; i++)
	{
		if (opt == opt_string[i])
		{
			if (opt_string[i + 1] == ':')
			{
				if (opt_string[i + 2] == ':')
					return (OPT_ARG);
				return (REQ_ARG);
			}
			return (NO_ARG);
		}
	}
	return (UNKNOWN);
}

int	ft_getopt_short(int argc, char **argv, const char *opt_string)
{
	static int	j = 1;
	short		type;

	if (g_optind >= argc || !argv[g_optind] || argv[g_optind][0] != '-'
		|| argv[g_optind][1] == '\0')
		return (-1);
	if (argv[g_optind][0] == '-' && argv[g_optind][1] == '-'
		&& argv[g_optind][2] == '\0')
	{
		g_optind++;
		return (-1);
	}
	g_optopt = argv[g_optind][j];
	type = gettype(g_optopt, opt_string);
	switch (type)
	{
	case UNKNOWN:
		if (g_opterr)
			fprintf(stderr, "%s: illegal option -- %c\n", argv[0], g_optopt);
		if (argv[g_optind][++j] == '\0')
		{
			g_optind++;
			j = 1;
		}
		return ('?');
	case NO_ARG:
		if (argv[g_optind][++j] == '\0')
		{
			g_optind++;
			j = 1;
		}
		return (g_optopt);
	case REQ_ARG:
		if (argv[g_optind][j + 1] != '\0')
			g_optarg = &argv[g_optind][j + 1];
		else if (g_optind + 1 < argc)
			g_optarg = argv[++g_optind];
		else
		{
			if (g_opterr)
				fprintf(stderr, "%s: option requires an argument -- %c\n",
					argv[0], g_optopt);
			g_optind++;
			j = 1;
			return ('?');
		}
		g_optind++;
		j = 1;
		return (g_optopt);
	case OPT_ARG:
		if (argv[g_optind][j + 1] != '\0')
			g_optarg = &argv[g_optind][j + 1];
		else
			g_optarg = NULL;
		g_optind++;
		j = 1;
		return (g_optopt);
	}
	return (-1);
}

int	ft_getopt_long(int argc, char **argv, const char *opt_string,
		const struct option *longopts, int *longindex)
{
	const char *arg;
	const char *eq;
	size_t	len;
	int		ret;

	if (g_optind >= argc || argv[g_optind] == NULL)
		return (-1);
	arg = argv[g_optind];
	if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
	{
		g_optind++;
		return (-1);
	}
	if (arg[0] != '-' || arg[1] == '\0')
		return (-1);
	if (arg[1] != '-')
		return (ft_getopt_short(argc, argv, opt_string));
	arg += 2;
	if (*arg == '\0')
		return (-1);
	eq = strchr(arg, '=');
	for (int i = 0; longopts && longopts[i].name; i++)
	{
		len = strlen(longopts[i].name);
		if ((eq && strncmp(arg, longopts[i].name, eq - arg) == 0
				&& (size_t)(eq - arg) == len) || (!eq && strcmp(arg,
					longopts[i].name) == 0))
		{
			if (longindex)
				*longindex = i;
			switch (longopts[i].has_arg)
			{
			case NO_ARG:
				if (eq)
				{
					if (g_opterr)
						fprintf(stderr, "option '--%s' doesn't allow an argument\n",
							longopts[i].name);
					g_optind++;
					return ('?');
				}
				break ;
			case REQ_ARG:
				if (eq)
					g_optarg = (char *)(eq + 1);
				else if (g_optind + 1 < argc)
					g_optarg = argv[++g_optind];
				else
				{
					if (g_opterr)
						fprintf(stderr, "option '--%s' requires an argument\n",
							longopts[i].name);
					g_optind++;
					return ('?');
				}
				break ;
			case OPT_ARG:
				if (eq)
					g_optarg = (char *)(eq + 1);
				else
					g_optarg = NULL;
				break ;
			}
			if (longopts[i].flag)
			{
				*longopts[i].flag = longopts[i].val;
				g_optind++;
				return (0);
			}
			else
			{
				ret = longopts[i].val;
				g_optind++;
				return (ret);
			}
		}
	}
	if (g_opterr)
		fprintf(stderr, "%s: unrecognized option '--%s'\n", argv[0], arg);
	g_optind++;
	return ('?');
}

int	ft_getopt_long_only(int argc, char **argv, const char *opt_string,
		const struct option *longopts, int *longindex)
{
	const char *arg = argv[g_optind] + 1;

	if (g_optind < argc && argv[g_optind][0] == '-' && argv[g_optind][1] != '-')
	{
		if (strchr(opt_string, arg[0]))
			return (ft_getopt_short(argc, argv, opt_string));
		for (int i = 0; longopts && longopts[i].name; i++)
		{
			if (strcmp(arg, longopts[i].name) == 0)
				return (ft_getopt_long(argc, argv, opt_string, longopts,
					longindex));
		}
	}
	return (ft_getopt_long(argc, argv, opt_string, longopts, longindex));
}

int	ft_getopt(int argc, char **argv, const char *optstring,
		const struct option *longopts, int *longindex, bool long_only)
{
	if (longopts)
	{
		if (long_only)
			return (ft_getopt_long_only(argc, argv, optstring, longopts,
				longindex));
		else
			return (ft_getopt_long(argc, argv, optstring, longopts, longindex));
	}
	return (ft_getopt_short(argc, argv, optstring));
}
