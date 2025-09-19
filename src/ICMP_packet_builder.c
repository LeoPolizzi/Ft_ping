/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ICMP_packet_builder.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/07 18:22:19 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/08 17:38:06 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static uint16_t icmp_checksum(uint16_t *buf, int len)
{
    uint32_t sum;

	sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1)
        sum += *((uint8_t *)buf);
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return ((uint16_t)(~sum));
}

bool	build_packet(uint16_t seq, struct icmp_packet **pack_ptr)
{
    struct timeval timestamp;
    unsigned long size = PING_DEFAULT_DATA_LEN;
	struct icmp_packet *packet = *pack_ptr;

	if (!pack_ptr)
		return (false);
	packet = calloc(1, sizeof(struct icmp_packet));
	if (!packet)
		return (false);
    if (data.opts.size > 0 && data.opts.size <= PING_MAX_DATA_LEN)
        size = data.opts.size;
    memset(&packet, 0, sizeof(packet));
    packet->hdr.type = ICMP_ECHO;
    packet->hdr.code = 0;
    packet->hdr.id = htons(getpid() & 0xFFFF);
    packet->hdr.sequence = htons(seq);
    gettimeofday(&timestamp, NULL);
    memcpy(packet->payload, &timestamp, sizeof(timestamp));
    if (sizeof(timestamp) < size)
        memset(packet->payload + sizeof(timestamp), 0x42, size - sizeof(timestamp));
    packet->hdr.checksum = 0;
    packet->hdr.checksum = icmp_checksum((uint16_t *)&packet, sizeof(packet));
    return (true);
}
