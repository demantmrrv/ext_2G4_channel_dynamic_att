/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _CHANNEL_DYNAMIC_ATT_COM_H
#define _CHANNEL_DYNAMIC_ATT_COM_H
#include "channel_dynamic_att_com_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open client communication
 *
 * @param sim_id The current SIM ID used for logical path for fifo
 * @param fifo_name The logical name of the fifo
 */
void channel_dynamic_att_com_open(char *sim_id, char *fifo_name);

/**
 * @brief Close client communication
*/
void channel_dynamic_att_com_close(void);

/**
 * @brief Read an entire packet from fifo
 *
 * @param packet Pointer to allocated memory for full packet structure
 * @return True if a packet was read and validated
*/
bool channel_dynamic_att_com_read_packet(ch_dynamic_att_com_protocol_packet_t *packet);

#ifdef __cplusplus
}
#endif

#endif /* _CHANNEL_DYNAMIC_ATT_COM_H */
