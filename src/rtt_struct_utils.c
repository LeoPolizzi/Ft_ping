/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rtt_struct_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lpolizzi <lpolizzi@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 13:16:24 by lpolizzi          #+#    #+#             */
/*   Updated: 2025/09/25 13:24:06 by lpolizzi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static struct rtt_node *create_node(struct timeval rtt)
{
	struct rtt_node *new_node = calloc(1, sizeof(struct rtt_node));
	if (!new_node)
		return (NULL);
	new_node->rtt = rtt;
	new_node->next = NULL;
	return (new_node);
}

void add_rtt(struct timeval rtt)
{
    struct rtt_node *new_node = create_node(rtt);
    if (!new_node)
        return;
    if (!data.packinfo.min)
    {
        data.packinfo.min = calloc(1, sizeof(struct timeval));
        *data.packinfo.min = rtt;
    }
    if (!data.packinfo.max)
    {
        data.packinfo.max = calloc(1, sizeof(struct timeval));
        *data.packinfo.max = rtt;
    }
    if (!data.packinfo.rtt_list)
    {
        data.packinfo.rtt_list = new_node;
        data.packinfo.rtt_last = new_node;
    }
    else
    {
        data.packinfo.rtt_last->next = new_node;
        data.packinfo.rtt_last = new_node;
    }
    if ((rtt.tv_sec < data.packinfo.min->tv_sec) || (rtt.tv_sec == data.packinfo.min->tv_sec && rtt.tv_usec < data.packinfo.min->tv_usec))
	{
		*data.packinfo.min = rtt;
	}
    if ((rtt.tv_sec > data.packinfo.max->tv_sec) || (rtt.tv_sec == data.packinfo.max->tv_sec && rtt.tv_usec > data.packinfo.max->tv_usec))
	{
		*data.packinfo.max = rtt;
	}
	double ms = rtt.tv_sec * 1000.0 + rtt.tv_usec / 1000.0;
	double avg_ms = data.packinfo.avg.tv_sec * 1000.0 + data.packinfo.avg.tv_usec / 1000.0;
	avg_ms = (avg_ms * (data.packinfo.nb_ok - 1) + ms) / data.packinfo.nb_ok;
	double mdev_ms = data.packinfo.mdev.tv_sec * 1000.0 + data.packinfo.mdev.tv_usec / 1000.0;
	mdev_ms = sqrt(((mdev_ms * mdev_ms * (data.packinfo.nb_ok - 1)) + (ms - avg_ms) * (ms - avg_ms)) / data.packinfo.nb_ok);
	data.packinfo.avg.tv_sec = (time_t)(avg_ms / 1000.0);
	data.packinfo.avg.tv_usec = (suseconds_t)((avg_ms - data.packinfo.avg.tv_sec * 1000.0) * 1000.0 + 0.5); // +0.5 to round
	data.packinfo.mdev.tv_sec = (time_t)(mdev_ms / 1000.0);
	data.packinfo.mdev.tv_usec = (suseconds_t)((mdev_ms - data.packinfo.mdev.tv_sec * 1000.0) * 1000.0 + 0.5);
}

void	free_list(struct rtt_node **head)
{
	struct rtt_node *current = *head;
	struct rtt_node *next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head = NULL;
}
