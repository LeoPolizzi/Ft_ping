/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_socket.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:17:24 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/25 13:17:26 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

bool init_socket(char *prog_name)
{
    int one = 1;

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
        }
        else
        {
            fprintf(stderr, "%s: %s\n", prog_name, strerror(errno));
            return (false);
        }
    }
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
		unsigned char rr[40];
		rr[0] = 7;
		rr[1] = 39;
		rr[2] = 4;
		memset(rr+3, 0, 36);
		if (setsockopt(data.sockinfo.sockfd, IPPROTO_IP, IP_OPTIONS, rr, sizeof(rr)) < 0)
		{
			fprintf(stderr, "%s: setsockopt IP_OPTIONS: %s\n", prog_name, strerror(errno));
			return (false);
		}
	}
    return (true);
}
