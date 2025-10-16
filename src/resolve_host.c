/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_host.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:16:53 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/28 17:10:24 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

bool	resolve_hostname(char *hostname)
{
	struct addrinfo	hints;
	struct addrinfo	*res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
	{
		fprintf(stderr, "%s: %s: No address associated with hostname\n", prog_name, hostname);
		return (false);
	}
	data.sockinfo.addr = *(struct sockaddr_in *)res->ai_addr;
	data.sockinfo.addr_len = res->ai_addrlen;
	data.sockinfo.hostname = strdup(hostname);
	if (inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, data.sockinfo.ip_str, sizeof(data.sockinfo.ip_str)) == NULL)
	{
		fprintf(stderr, "%s: inet_ntop: %s\n", prog_name, strerror(errno));
		freeaddrinfo(res);
		return (false);
	}
	freeaddrinfo(res);
	return (true);
}
