/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:17:52 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/30 16:29:17 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static bool	send_ping()
{
	static int	sequence = 0;

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
	gettimeofday(&end_time, NULL);
	last_sent = end_time;
	free(data.packinfo.packet);
	data.packinfo.nb_send++;
	sequence++;
	return (true);
}

static bool receive_ping()
{
    static bool seq_seen[MAXSEQ] = {0};
    unsigned char buf[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    ssize_t bytes_received;
    struct timeval current, diff;
    struct ip *ip_hdr;
    size_t ip_hdr_len;
    struct icmp_hdr *icmp_hdr;
    uint16_t seq = 0;
    int ttl;

    bytes_received = recvfrom(data.sockinfo.sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sender, &sender_len);
    if (bytes_received < 0)
    {
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            fprintf(stderr, "%s: recvfrom: %s\n", prog_name, strerror(errno));
        return false;
    }
    gettimeofday(&current, NULL);
    timersub(&current, &last_sent, &diff);
    ip_hdr = (struct ip *)buf;
    ip_hdr_len = ip_hdr->ip_hl * 4;
    if (bytes_received < (ssize_t)(ip_hdr_len + sizeof(struct icmp_hdr)))
        return false;
    icmp_hdr = (struct icmp_hdr *)(buf + ip_hdr_len);
    ttl = ip_hdr->ip_ttl;
    switch (icmp_hdr->type)
    {
        case ICMP_ECHOREPLY:
            seq = ntohs(icmp_hdr->sequence);
            if (seq < MAXSEQ)
            {
                if (seq_seen[seq])
                    data.packinfo.nb_dup++;
                if (!(data.opts.opt_mask & OPT_QUIET))
                {
                    fprintf(stdout, "%ld bytes from %s: icmp_seq=%d ttl=%d",
                            bytes_received - ip_hdr_len,
                            inet_ntoa(sender.sin_addr),
                            seq, ttl);
                    if (data.opts.size > 15)
                        fprintf(stdout, " time=%.3f ms", diff.tv_sec * 1e3 + diff.tv_usec / 1e3);
                    fprintf(stdout, "%s\n", seq_seen[seq] ? " (DUP!)" : "");
                }
            }
            break;
        case ICMP_TIME_EXCEEDED:
        {
            struct ip *inner_ip = (struct ip *)(buf + ip_hdr_len + sizeof(struct icmp_hdr));
            struct icmp_hdr *inner_icmp = NULL;
            if ((bytes_received - ip_hdr_len - sizeof(struct icmp_hdr)) >= sizeof(struct ip) + 8)
            {
                inner_icmp = (struct icmp_hdr *)((unsigned char *)inner_ip + inner_ip->ip_hl * 4);
                seq = ntohs(inner_icmp->sequence);
            }

            if (!(data.opts.opt_mask & OPT_QUIET))
            {
                fprintf(stdout, "%ld bytes from %s: ICMP Time Exceeded (ttl expired)",
                        bytes_received - ip_hdr_len,
                        inet_ntoa(sender.sin_addr));
                if (inner_icmp)
                    fprintf(stdout, " original ttl=%d, icmp_seq=%d", inner_ip->ip_ttl, seq);
                fprintf(stdout, "\n");
            }
            break;
        }
        default:
            break;
    }
	seq_seen[seq] = 1;
	data.packinfo.nb_ok++;
    add_rtt(diff);
    return true;
}

void ping_loop(void)
{
    struct timeval send_time, now, elapsed, wait, timeout;
    fd_set readfds;
    int ret;

    signal(SIGINT, sigint_handler);
	gettimeofday(&start_time, NULL);
	last_sent = start_time;
    fprintf(stdout, "PING %s (%s) %d data bytes%s",
        data.sockinfo.hostname,
        data.sockinfo.ip_str,
        data.opts.size, (data.opts.opt_mask & OPT_VERBOSE) ? "" : "\n");
	if (data.opts.opt_mask & OPT_VERBOSE)
		fprintf(stdout, ", id 0x%x = %d\n", g_icmp_id, g_icmp_id);
    gettimeofday(&send_time, NULL);
    for (int i = 0; i < data.opts.preload; i++)
        send_ping();
    if (!send_ping())
	{
		return;
	}
	if (data.opts.timeout > 0)
	{
		gettimeofday(&timeout, NULL);
		timeout.tv_sec += data.opts.timeout;
	}
    while (!stop)
    {
        FD_ZERO(&readfds);
        FD_SET(data.sockinfo.sockfd, &readfds);
        wait.tv_sec = 1;
        wait.tv_usec = 0;
        ret = select(data.sockinfo.sockfd + 1, &readfds, NULL, NULL, &wait);
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
		if (data.opts.count > 0 && data.packinfo.nb_send >= data.opts.count)
			break;
        gettimeofday(&now, NULL);
		if (data.opts.timeout > 0 && timercmp(&now, &timeout, >=))
			break;
        timersub(&now, &send_time, &elapsed);
        if (elapsed.tv_sec >= 1)
        {
            gettimeofday(&send_time, NULL);
            if (!send_ping())
                break;
        }
    }
    if (data.opts.linger > 0 && !stop)
    {
        struct timeval linger_elapsed;
        gettimeofday(&now, NULL);
        timersub(&now, &last_sent, &linger_elapsed);
        while (linger_elapsed.tv_sec < data.opts.linger)
        {
            if (data.opts.count > 0 && data.packinfo.nb_ok >= data.opts.count)
                break;
            FD_ZERO(&readfds);
            FD_SET(data.sockinfo.sockfd, &readfds);
            wait.tv_sec = data.opts.linger - linger_elapsed.tv_sec;
            wait.tv_usec = 0;
            ret = select(data.sockinfo.sockfd + 1, &readfds, NULL, NULL, &wait);
            if (ret > 0 && FD_ISSET(data.sockinfo.sockfd, &readfds))
                receive_ping();
            gettimeofday(&now, NULL);
            timersub(&now, &last_sent, &linger_elapsed);
        }
    }
    ending_stats();
}
