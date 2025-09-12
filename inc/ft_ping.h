/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 17:48:48 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/08 17:37:55 by lpolizzi         ###   ########.fr       */
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

# include "lib_getopt.h"

# define VERSION "1.0.0"

# define USAGE_MESSAGE \
"\t-c, --count <count>\tStop after sending (and receiving) <count> ECHO_RESPONSE packets\n\
\t-d, --debug\t\tSet the SO_DEBUG option on the socket being used\n\
\t-f, --flood\t\tFlood ping (may be hard on network; root only)\n\
\t-i, --interval <wait>\tWait <wait> seconds between sending each packet\n\
\t-l, --preload <preload>\tSend <preload> packets as fast as possible before normal mode\n\
\t-n, --numeric\t\tNumeric output only (no symbolic name resolution)\n\
\t-p, --pattern <pattern>\tSpecify up to 16 bytes to fill the packet\n\
\t-q, --quiet\t\tQuiet output (only summary lines shown)\n\
\t-R, --route\t\tRecord route and display it on returned packets\n\
\t-r, --ignore-routing\tBypass routing tables; send directly on attached network\n\
\t-s, --size <packetsize>\tNumber of data bytes to send (default 56)\n\
\t-v, --verbose\t\tVerbose output (other ICMP packets shown)\n\
\t-t, --ttl <N>\t\tSet packet time-to-live\n\
\t-T, --tos <num>\t\tSet packet type of service (TOS)\n\
\t-w, --timeout <N>\tStop after <N> seconds\n\
\t-W, --linger <N>\tNumber of seconds to wait for response\n\
\t-?, --help\t\tDisplay this help message\n\
\t-V, --version\t\tPrint program version\n\
\t--address\t\tSend ICMP_ADDRESS packets (root only)\n\
\t--mask\t\t\tSame as --address\n\
\t--echo\t\t\tSend ICMP_ECHO packets (default)\n\
\t--timestamp\t\tSend ICMP_TIMESTAMP packets\n\
\t--ip-timestamp <flag>\tIP timestamp type: \"tsonly\" or \"tsaddr\"\n\
\t--usage\t\t\tDisplay a short usage message\n"

# define DEFAULT_PAYLOAD_SIZE 56
# define MAX_PAYLOAD_SIZE 65507

#define OPT_FLOOD       0x001
#define OPT_INTERVAL    0x002
#define OPT_NUMERIC     0x004
#define OPT_QUIET       0x008
#define OPT_RROUTE      0x010
#define OPT_VERBOSE     0x020
#define OPT_IPTIMESTAMP 0x040

// Still need to figure these options
// #define OPT_FLOWINFO    0x080
// #define OPT_TCLASS      0x100

extern struct pingdata data;

enum opts {
	L_OPT_ADDRESS = 1,
	L_OPT_MASK,
	L_OPT_ECHO,
	L_OPT_TIMESTAMP,
	L_OPT_IP_TIMESTAMP,
	L_OPT_USAGE
};

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
    uint8_t payload[MAX_PAYLOAD_SIZE];
};

struct sockinfo
{
	int sockfd;
	char *hostname;
	struct sockaddr_in addr;
	char ip_str[INET_ADDRSTRLEN];
};

struct packinfo {
	int nb_send;
	int nb_ok;
	struct timeval *min;
	struct timeval *max;
	struct timeval avg;
	struct timeval stddev;
	struct rtt_node *rtt_list;
	struct rtt_node *rtt_last;
};

struct pingopts
{
	bool address;                        // --address
	bool mask;                           // --mask
	bool echo;                           // --echo
	int count;							 // -c, --count
    double interval;					 // -i, --interval
    int preload;					     // -l, --preload
    char *pattern;						 // -p, --pattern
    int size;							 // -s, --size
    int ttl;			                 // --ttl
    int tos;							 // -T, --tos
    int timeout;						 // -w, --timeout
    int linger;						     // -W, --linger
    bool timestamp;						 // --timestamp
	int sock_flags;                      // Socket flags (SO_DEBUG, SO_DONTROUTE)
	int opt_mask;                        // Bitmask for options
};

struct pingdata
{
	struct sockinfo sockinfo;
	struct pingopts opts;
};

void parse_opts(int ac, char **av);
void stop_ping(int exit_code);
void help_message(char *prog_name);
struct icmp_packet build_packet(uint16_t seq);

#endif
