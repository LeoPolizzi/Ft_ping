/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 17:48:48 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/30 16:29:37 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/time.h>
# include <math.h>
# include <limits.h>

# include "lib_getopt.h"

# define MAYBE_UNUSED __attribute__((unused))

# define VERSION "1.0.0"

# define USAGE "\
[-dnrvqR?V] [-c NUMBER] [-w N] [-W N] [-l NUMBER] [-s NUMBER]\n\
\t\t [--count=NUMBER] [--debug] [--numeric] [--ignore-routing] [--ttl=N]\n\
\t\t [--verbose] [--timeout=N] [--linger=N] [--preload=NUMBER]\n\
\t\t [--quiet] [--route] [--size=NUMBER] [--help] [--usage] [--version]\n\
\t\t HOST ...\n\
"

# define HELP_MESSAGE "\
Send ICMP ECHO_REQUEST packets to network hosts.\n\
\n\
Options:\n\
\n\
\t-c, --count=NUMBER         stop after sending NUMBER packets\n\
\t-d, --debug                set the SO_DEBUG option\n\
\t-n, --numeric              do not resolve host addresses\n\
\t-r, --ignore-routing       send directly to a host on an attached network\n\
\t--ttl=N                    specify N as time-to-live\n\
\t-v, --verbose              verbose output\n\
\t-w, --timeout=N            stop after N seconds\n\
\t-W, --linger=N             number of seconds to wait for response\n\
\n\
\t-l, --preload=NUMBER       send NUMBER packets as fast as possible before\n\
\t                           falling into normal mode of behavior (root only)\n\
\t-q, --quiet                quiet output\n\
\t-R, --route                record route\n\
\t-s, --size=NUMBER          send NUMBER data octets\n\
\n\
\t-?, --help                 give this help list\n\
\t--usage                    give a short usage message\n\
\t-V, --version              print program version\n\
\n\
Mandatory or optional arguments to long options are also mandatory or optional\n\
for any corresponding short options.\n\
\n\
Options marked with (root only) are available only to superuser.\n\
\n\
Report bugs to <lpolizzi@student.42nice.fr>.\n"

# define SECOND_IN_USEC 1000000

# define MAXSEQ 65535

# define MAXIPLEN    60
# define MAXICMPLEN  76

# define PING_DEFAULT_DATA_LEN 56
# define PING_MAX_DATA_LEN (65507 - MAXIPLEN - MAXICMPLEN)

# define L_OPT_USAGE	1
# define L_OPT_TTL		2

# define OPT_NUMERIC     0x001
# define OPT_QUIET       0x002
# define OPT_RROUTE      0x004
# define OPT_VERBOSE     0x008

extern struct pingdata data;
extern volatile bool stop;
extern struct timeval last_sent, start_time, end_time;
extern char *prog_name;
extern bool is_root;
extern int	icmp_id;

struct icmp_hdr
{
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

struct icmp_packet
{
    struct icmp_hdr hdr;
    uint8_t payload[PING_MAX_DATA_LEN];
};

struct sockinfo
{
	int sockfd;
	char *hostname;
	struct sockaddr_in addr;
	socklen_t addr_len;
	char ip_str[INET_ADDRSTRLEN];
};

struct rtt_node
{
	struct timeval rtt;
	struct rtt_node *next;
};

struct packinfo
{
	struct icmp_packet *packet;
	int nb_send;
	int nb_ok;
	int nb_dup;
	struct timeval *min;
	struct timeval *max;
	struct timeval avg;
	struct timeval mdev;
	struct rtt_node *rtt_list;
	struct rtt_node *rtt_last;
};

struct pingopts
{
	int count;							// -c, --count
    int size;							// -s, --size
	int preload;						// -p, --preload
	int ttl;							// -t, --ttl
    int timeout;						// -w, --timeout
    int linger;							// -W, --linger
	int sock_flags;					    // Socket flags (SO_DEBUG, SO_DONTROUTE)
	int opt_mask;					    // Bitmask for options
};

struct pingdata
{
	struct sockinfo sockinfo;
	struct pingopts opts;
	struct packinfo packinfo;
};

void parse_opts(int ac, char **av);
void stop_ping(int exit_code);
void help_message(char *prog_name);
bool build_packet(uint16_t seq, struct icmp_packet **pack_ptr);
bool init_socket(char *prog_name);
bool resolve_hostname(char *prog_name, char *hostname);
void ping_loop(void);
void free_list(struct rtt_node **head);
void sigint_handler(int signal MAYBE_UNUSED);
void add_rtt(struct timeval rtt);
void ending_stats();

#endif
