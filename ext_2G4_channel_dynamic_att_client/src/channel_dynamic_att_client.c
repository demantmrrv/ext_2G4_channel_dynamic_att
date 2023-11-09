/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <fcntl.h>
#include <string.h>
#include "bs_oswrap.h"
#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_pc_base.h"
#include "channel_dynamic_att_client.h"
#include "channel_dynamic_att_com_protocol.h"
#include "channel_dynamic_att_defaults.h"

extern char *pb_com_path;
extern uint global_device_nbr;

static struct {
    char *fifo_full_path;
    int   fifo_write_handle;
} channel_dynamic_att_client_prv = {0};

static bool channel_dynamic_att_client_write(void *data, size_t data_size)
{
    ssize_t bytes_written;

    if (channel_dynamic_att_client_prv.fifo_write_handle < 0) {
        return false;
    }

    bytes_written = write(channel_dynamic_att_client_prv.fifo_write_handle, data, data_size);
    if (bytes_written == -1) {
        bs_trace_error("Failed writing fifo");
    }
    return bytes_written == data_size;
}

static bool channel_dynamic_att_client_write_cmd(unsigned short command, void *payload, size_t payload_size)
{
    ch_dynamic_att_com_protocol_packet_t packet = {0};

    packet.header.command = command;
    packet.header.payload_size = payload_size;
    if (payload && payload_size) {
        memcpy((void *)&packet.payload, payload, payload_size);
    }

    bs_trace_raw(8, "channel_dynamic_att_client_write_cmd: Sending cmd=%u, data size=%u\n", command, payload_size);

    return channel_dynamic_att_client_write(&packet, sizeof(ch_dynamic_att_com_protocol_header_t) + packet.header.payload_size);
}

bool channel_dynamic_att_client_open(char *fifo_name)
{
    if (!fifo_name) {
        fifo_name = DYNAMIC_ATT_DEFAULT_FIFO_NAME;
    }

    channel_dynamic_att_client_prv.fifo_full_path = bs_calloc(strlen(pb_com_path) + strlen(fifo_name) + 2, sizeof(char));
    if (!channel_dynamic_att_client_prv.fifo_full_path) {
        bs_trace_error("Error allocating memory for fifo path");
    }
    sprintf(channel_dynamic_att_client_prv.fifo_full_path, "%s/%s", pb_com_path, fifo_name);

    if ((channel_dynamic_att_client_prv.fifo_write_handle = open(channel_dynamic_att_client_prv.fifo_full_path, O_WRONLY)) == -1) {
        bs_trace_error("Failed opening fifo for writing");
    }
}

void channel_dynamic_att_client_close()
{
    if (channel_dynamic_att_client_prv.fifo_write_handle >= 0) {
        close(channel_dynamic_att_client_prv.fifo_write_handle);
        channel_dynamic_att_client_prv.fifo_write_handle = -1;
    }

    if (channel_dynamic_att_client_prv.fifo_full_path) {
        free(channel_dynamic_att_client_prv.fifo_full_path);
        channel_dynamic_att_client_prv.fifo_full_path = NULL;
    }
}

bool channel_dynamic_att_client_reset(void)
{
    return channel_dynamic_att_client_write_cmd(DYNAMIC_ATT_PROTOCOL_CMD_RESET, NULL, 0);
}

bool channel_dynamic_att_client_set_attenuation_all(double attentuation)
{
    ch_dynamic_att_com_protocol_set_att_all_t payload = {
        .device      = global_device_nbr,
        .attenuation = attentuation
    };

    return channel_dynamic_att_client_write_cmd(DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL, &payload, sizeof(ch_dynamic_att_com_protocol_set_att_all_t));
}

bool channel_dynamic_att_client_set_attenuation_one(unsigned short peer_device, double attentuation_rx, double attentuation_tx)
{
    ch_dynamic_att_com_protocol_set_att_one_t payload = {
        .device         = global_device_nbr,
        .peer_device    = peer_device,
        .attenuation_rx = attentuation_rx,
        .attenuation_tx = attentuation_tx
    };

    return channel_dynamic_att_client_write_cmd(DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ONE, &payload, sizeof(ch_dynamic_att_com_protocol_set_att_one_t));
}
