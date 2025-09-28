/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_socket.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:17:24 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/28 17:33:20 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

bool init_socket(char *prog_name)
{
    int one = 1;
	char rspace[MAX_IPOPTLEN];

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
			else if (data.opts.opt_mask & OPT_VERBOSE)
					fprintf(stderr, "%s: sock4.fd: %d (socktype: SOCK_DGRAM), sock6.fd: Not Implemented, ", prog_name, data.sockinfo.sockfd);
        }
        else
        {
            fprintf(stderr, "%s: %s\n", prog_name, strerror(errno));
            return (false);
        }
    }
	else if (data.opts.opt_mask & OPT_VERBOSE)
   			fprintf(stderr, "%s: sock4.fd: %d (socktype: SOCK_RAW), sock6.fd: Not Implemented, ", prog_name, data.sockinfo.sockfd);
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
		memset(rspace, 0, sizeof (rspace));
		rspace[IPOPT_OPTVAL] = IPOPT_RR;
		rspace[IPOPT_OLEN] = sizeof (rspace) - 1;
		rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		if (setsockopt(data.sockinfo.sockfd, IPPROTO_IP, IP_OPTIONS, rspace, sizeof (rspace)) < 0)
		{
			fprintf(stderr, "%s: setsockopt IP_OPTIONS: %s\n", prog_name, strerror(errno));
			return (false);
		}
	}
    return (true);
}
