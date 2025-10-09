/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_getopt.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/30 17:07:23 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/08/30 18:04:10 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_GETOPT_H
# define FT_GETOPT_H

# include <stdbool.h>
# include <stddef.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>

extern int	g_optind,
			g_opterr,
			g_optopt;

extern char	*g_optarg;

struct option
{
	const char *name;
	int         has_arg;
	int        *flag;
	int         val;
};

enum {
	NO_ARG = 0,
	REQ_ARG = 1,
	OPT_ARG = 2,
	UNKNOWN = -1
};

/* GETOPT Dispatcher */
int	ft_getopt(int argc, char **argv, const char *optstring,
		const struct option *longopts, int *longindex, bool long_only);

/* GETOPT Short Options */
int	ft_getopt_short(int argc, char **argv, const char *opt_string);

/* GETOPT Long Options */
int	ft_getopt_long(int argc, char **argv, const char *opt_string,
				   const struct option *longopts, int *longindex);

/* GETOPT Long Options (only) */
int	ft_getopt_long_only(int argc, char **argv, const char *opt_string,
		const struct option *longopts, int *longindex);

#endif
