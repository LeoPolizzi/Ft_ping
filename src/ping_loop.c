/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:17:52 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/28 17:28:20 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static bool	send_ping()
{
	static int	sequence = 1;

	if (data.opts.count > 0 && data.packinfo.nb_send >= data.opts.count)
	{
		stop = true;
		return (true);
	}
	if (!build_packet(sequence, &data.packinfo.packet))
		return (false);
	if (sendto(data.sockinfo.sockfd, data.packinfo.packet, sizeof(data.packinfo.packet->hdr) + data.opts.size, 0, (struct sockaddr *)&data.sockinfo.addr, sizeof(data.sockinfo.addr)) <= 0)
	{
		fprintf(stderr, "%s: sendto: %s\n", prog_name, strerror(errno));
		free(data.packinfo.packet);
		return (false);
	}
	gettimeofday(&last_sent, NULL);
	free(data.packinfo.packet);
	data.packinfo.nb_send++;
	sequence++;
	return (true);
}

static bool receive_ping()
{
	static bool seq_seen[MAXSEQ] = {0};
	unsigned char	buf[1024];
	struct sockaddr_in	sender;
	socklen_t		sender_len = sizeof(sender);
	ssize_t			bytes_received;
	struct timeval	current;
	struct timeval	diff;
	struct ip *ip_hdr;
	size_t ip_hdr_len;
	struct icmp_hdr *icmp_hdr;
	uint16_t	seq;

	bytes_received = recvfrom(data.sockinfo.sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sender, &sender_len);
	if (bytes_received < 0)
	{
		if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
			fprintf(stderr, "%s: recvfrom: %s\n", prog_name, strerror(errno));
		return (false);
	}
	gettimeofday(&current, NULL);
	timersub(&current, &last_sent, &diff);
	ip_hdr = (struct ip *)buf;
	ip_hdr_len = ip_hdr->ip_hl * 4;
	if (bytes_received < (ssize_t)(ip_hdr_len + sizeof(struct icmp_hdr)))
		return (false);
	icmp_hdr = (struct icmp_hdr *)(buf + ip_hdr_len);
	seq = ntohs(icmp_hdr->sequence);
	if (seq < MAXSEQ)
	{
		if (seq_seen[seq])
		{
			data.packinfo.nb_dup++;
			if (!(data.opts.opt_mask & OPT_QUIET))
				fprintf(stdout, "%ld bytes from %s: icmp_seq=%u, time=%.2f ms (DUP!)\n", bytes_received, inet_ntoa(sender.sin_addr), seq, diff.tv_sec * 1e3 + diff.tv_usec / 1e3);
		}
		else
		{
			seq_seen[seq] = 1;
			if (!(data.opts.opt_mask & OPT_QUIET))
				fprintf(stdout, "%ld bytes from %s: icmp_seq=%d, time=%.2f ms\n", bytes_received, inet_ntoa(sender.sin_addr), seq, (diff.tv_sec * 1e3) + (diff.tv_usec / 1e3));
		}
		data.packinfo.nb_ok++;
	}
	add_rtt(diff);
	return (true);
}

void ping_loop(void)
{
    struct timeval timeout;
    fd_set readfds;
    struct timeval send_time, now, elapsed;
    struct timeval interval;
    signal(SIGINT, sigint_handler);
    gettimeofday(&start_time, NULL);
    last_sent = start_time;
    fprintf(stdout, "PING %s (%s) %d(%ld) bytes of data.\n",
        data.sockinfo.hostname,
        data.sockinfo.ip_str,
        (int)(sizeof(data.packinfo.packet->hdr) + data.opts.size - 8),
        (long)(sizeof(data.packinfo.packet->hdr) + data.opts.size + 28));
    interval.tv_sec = 1;
    interval.tv_usec = 0;
    gettimeofday(&send_time, NULL);
	for (int i = 0; i < data.opts.preload; i++)
		send_ping();
    if (!send_ping())
        return;
    while (!stop)
    {
        FD_ZERO(&readfds);
        FD_SET(data.sockinfo.sockfd, &readfds);
        gettimeofday(&now, NULL);
        timersub(&send_time, &now, &elapsed);
        timeout.tv_sec = interval.tv_sec - elapsed.tv_sec;
        timeout.tv_usec = interval.tv_usec - elapsed.tv_usec;
        if (timeout.tv_usec < 0)
        {
            timeout.tv_usec += 1000000;
            timeout.tv_sec--;
        }
        if (timeout.tv_sec < 0)
            timeout.tv_sec = timeout.tv_usec = 0;
        int ret = select(data.sockinfo.sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            if (errno != EINTR)
            {
                fprintf(stderr, "%s: select: %s\n", prog_name, strerror(errno));
                stop_ping(EXIT_FAILURE);
            }
            continue;
        }
        if (ret > 0 && FD_ISSET(data.sockinfo.sockfd, &readfds))
            receive_ping();
        gettimeofday(&now, NULL);
        timersub(&now, &send_time, &elapsed);
        if (elapsed.tv_sec >= interval.tv_sec)
        {
            gettimeofday(&send_time, NULL);
            if (!send_ping())
                break;
        }
    }
    if (!stop)
        sleep(data.opts.linger);
    ending_stats();
}
