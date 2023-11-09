/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
** Include this file before including system headers.  By default, with
** C99 support from the compiler, it requests POSIX 2008 support.  With
** C89 support only, it requests POSIX 1997 support.  Override the
** default behaviour by setting either _XOPEN_SOURCE or _POSIX_C_SOURCE.
*/
/* _XOPEN_SOURCE 700 is loosely equivalent to _POSIX_C_SOURCE 200809L */
/* _XOPEN_SOURCE 600 is loosely equivalent to _POSIX_C_SOURCE 200112L */
/* _XOPEN_SOURCE 500 is loosely equivalent to _POSIX_C_SOURCE 199506L */
#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
#if defined(__cplusplus)
#define _XOPEN_SOURCE 700   /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#elif __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 700   /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#else
#define _XOPEN_SOURCE 500   /* SUS v2, POSIX 1003.1 1997 */
#endif /* __STDC_VERSION__ */
#endif /* !_XOPEN_SOURCE && !_POSIX_C_SOURCE */
#include <string.h>
#include <fcntl.h>
#include "bs_oswrap.h"
#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_pc_base.h"
#include "channel_dynamic_att_com.h"
#include "channel_dynamic_att_com_protocol.h"

extern char *pb_com_path;

static struct {
    char *fifo_full_path;
    int   fifo_read_handle;
} ch_dynamic_att_com_prv = {0};

static void channel_dynamic_att_com_prepare(char *sim_id, char *fifo_name)
{
    int folder_length = 0;

    if ((folder_length = pb_create_com_folder(sim_id)) <= 0) {
        bs_trace_error("Cannot create folder %s for communication fifo\n", sim_id);
    }

    ch_dynamic_att_com_prv.fifo_full_path = bs_calloc(folder_length + strlen(fifo_name) + 2, sizeof(char));
    if (!ch_dynamic_att_com_prv.fifo_full_path) {
        bs_trace_error("Error allocating memory for fifo path");
    }
    sprintf(ch_dynamic_att_com_prv.fifo_full_path, "%s/%s", pb_com_path, fifo_name);
}

void channel_dynamic_att_com_open(char *sim_id, char *fifo_name)
{
    if (!ch_dynamic_att_com_prv.fifo_full_path) {
        channel_dynamic_att_com_prepare(sim_id, fifo_name);
        if (pb_create_fifo_if_not_there(ch_dynamic_att_com_prv.fifo_full_path) != 0) {
            bs_trace_error("Failed creating fifo at location %s\n", ch_dynamic_att_com_prv.fifo_full_path);
        }
        ch_dynamic_att_com_prv.fifo_read_handle = open(ch_dynamic_att_com_prv.fifo_full_path, O_RDONLY | O_NONBLOCK);
        if (ch_dynamic_att_com_prv.fifo_read_handle < 0) {
            bs_trace_error("Failed opening fifo at location %s: %d\n", ch_dynamic_att_com_prv.fifo_full_path, ch_dynamic_att_com_prv.fifo_read_handle);
        }
        bs_trace_raw(8, "channel_dynamic_att opened fifo\n");
    }
}

void channel_dynamic_att_com_close(void)
{
    if (ch_dynamic_att_com_prv.fifo_full_path) {
        if (ch_dynamic_att_com_prv.fifo_read_handle >= 0) {
            close(ch_dynamic_att_com_prv.fifo_read_handle);
        }

        remove(ch_dynamic_att_com_prv.fifo_full_path);
        free(ch_dynamic_att_com_prv.fifo_full_path);
        ch_dynamic_att_com_prv.fifo_full_path = NULL;
    }
}

static bool channel_dynamic_att_com_read(size_t data_size, void *destination)
{
    ssize_t bytes_read;

    bytes_read = read(ch_dynamic_att_com_prv.fifo_read_handle, destination, data_size);
    if (bytes_read > 0) {
        bs_trace_raw(8, "channel_dynamic_att_com_read: Received %d bytes\n", bytes_read);
    }

    return bytes_read == data_size;
}

static bool channel_dynamic_att_com_read_cmd_data_set_att_all(ch_dynamic_att_com_protocol_packet_t *packet)
{
    if (packet->header.payload_size == sizeof(ch_dynamic_att_com_protocol_set_att_all_t)) {
        return channel_dynamic_att_com_read(sizeof(ch_dynamic_att_com_protocol_set_att_all_t), &packet->payload.set_att_all_payload);
    }
    bs_trace_error("Failed to read DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL data. Got %u bytes, but expected %u bytes\n", packet->header.payload_size, sizeof(ch_dynamic_att_com_protocol_set_att_all_t));
    return false;
}

static bool channel_dynamic_att_com_read_cmd_data_set_att_one(ch_dynamic_att_com_protocol_packet_t *packet)
{
    if (packet->header.payload_size == sizeof(ch_dynamic_att_com_protocol_set_att_one_t)) {
        return channel_dynamic_att_com_read(sizeof(ch_dynamic_att_com_protocol_set_att_one_t), &packet->payload.set_att_one_payload);
    }
    bs_trace_error("Failed to read DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL data. Got %u bytes, but expected %u bytes\n", packet->header.payload_size, sizeof(ch_dynamic_att_com_protocol_set_att_all_t));
    return false;
}

bool channel_dynamic_att_com_read_packet(ch_dynamic_att_com_protocol_packet_t *packet)
{
    bool return_val = false;

    if (packet != NULL) {
        if (channel_dynamic_att_com_read(sizeof(packet->header), &packet->header)) {
            switch (packet->header.command) {
                case DYNAMIC_ATT_PROTOCOL_CMD_RESET:
                    return_val = true;
                    break;
                case DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL:
                    return_val = channel_dynamic_att_com_read_cmd_data_set_att_all(packet);
                    break;
                case DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ONE:
                    return_val = channel_dynamic_att_com_read_cmd_data_set_att_one(packet);
                    break;
                default:
                    bs_trace_warning_line("Received unknown attenuation command: %u\n", packet->header.command);
            }
        }
    }
    return return_val;
}
