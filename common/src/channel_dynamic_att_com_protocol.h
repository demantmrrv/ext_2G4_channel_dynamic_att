/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H
#define _CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#define DYNAMIC_ATT_PROTOCOL_CMD_RESET       (0x0000)
#define DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL (0x0001)
#define DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ONE (0x0002)

/**
 * Command format:
 * Header:
 *   Bytes XX.. : Command word
 *   Bytes ..XX : Command payload data size in bytes excluding header
 *
 * Payload:
 *   CMD_RESET: None
 */
typedef struct {
    unsigned short command;
    unsigned short payload_size;
} __attribute__((packed)) ch_dynamic_att_com_protocol_header_t;

/**
 * @brief DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL
 *
 * Set same Rx and Tx attenuation between caller and all other devices
 *
 * Data size: 10 bytes
 *   Bytes XX........ : Callers device number
 *   Bytes ..XXXXXXXX : Attenuation for all connections
*/
typedef struct {
    unsigned short device;
    double         attenuation;
} __attribute__((packed)) ch_dynamic_att_com_protocol_set_att_all_t;

/**
 * @brief DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ONE:
 *
 * Set unique Rx and Tx attenuation between caller and one other peer device
 *
 * Data size: 20 bytes
 *   Bytes XX.................. : Callers device number
 *   Bytes ..XX................ : Peer device number
 *   Bytes ....XXXXXXXX........ : Rx attenuation for packets sent from peer to caller
 *   Bytes ............XXXXXXXX : Tx attenuation for packets sent from caller to peer
 */
typedef struct {
    unsigned short device;
    unsigned short peer_device;
    double         attenuation_rx;
    double         attenuation_tx;
} __attribute__((packed)) ch_dynamic_att_com_protocol_set_att_one_t;

/**
 * Combined packet structure
*/
typedef struct {
    ch_dynamic_att_com_protocol_header_t header;
    union
    {
        ch_dynamic_att_com_protocol_set_att_all_t set_att_all_payload;
        ch_dynamic_att_com_protocol_set_att_one_t set_att_one_payload;
    } payload;
} __attribute__((packed)) ch_dynamic_att_com_protocol_packet_t;

#ifdef __cplusplus
}
#endif

#endif /* _CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H */
