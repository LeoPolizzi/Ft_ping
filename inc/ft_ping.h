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

# define USAGE_MESSAGE "\
\t-c, --count <count>\tStop after sending (and receiving) <count> ECHO_RESPONSE packets\n\
\t-d, --debug\t\tSet the SO_DEBUG option on the socket being used\n\
\t-l, --preload <N>\tSend <N> packets as fast as possible before waiting for replies\n\
\t-n, --numeric\t\tNumeric output only (no symbolic name resolution)\n\
\t-q, --quiet\t\tQuiet output (only summary lines shown)\n\
\t-R, --route\t\tRecord route and display it on returned packets (Can cause responses to never be received since this implementation is based on IPv4)\n\
\t-r, --ignore-routing\tBypass routing tables; send directly on attached network (Can cause sento() errors since this implementation is based on IPv4)\n\
\t-s, --size <packetsize>\tNumber of data bytes to send (default 56)\n\
\t-v, --verbose\t\tVerbose output (other ICMP packets shown)\n\
\t-w, --timeout <N>\tStop after <N> seconds\n\
\t-W, --linger <N>\tNumber of seconds to wait for response\n\
\t-?, --help\t\tDisplay this help message\n\
\t-V, --version\t\tPrint program version\n\
\t--usage\t\t\tDisplay a short usage message\n"

# define SECOND_IN_USEC 1000000

# define MAXSEQ 65535

# define MAXIPLEN    60
# define MAXICMPLEN  76

# define PING_DEFAULT_DATA_LEN 56
# define PING_MAX_DATA_LEN (65507 - MAXIPLEN - MAXICMPLEN)

# define L_OPT_USAGE 1

# define OPT_NUMERIC     0x001
# define OPT_QUIET       0x002
# define OPT_RROUTE      0x004
# define OPT_VERBOSE     0x008

extern struct pingdata data;
extern volatile bool stop;
extern struct timeval last_sent, start_time;
extern char *prog_name;
extern bool first_received;

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
	int count;							 // -c, --count
    int size;							 // -s, --size
	int preload;                         // -p, --preload
    int timeout;						 // -w, --timeout
    int linger;						     // -W, --linger
	int sock_flags;                      // Socket flags (SO_DEBUG, SO_DONTROUTE)
	int opt_mask;                        // Bitmask for options
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
