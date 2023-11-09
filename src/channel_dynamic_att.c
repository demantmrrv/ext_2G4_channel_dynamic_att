/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <ctype.h>
#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "p2G4_pending_tx_rx_list.h"
#include "channel_dynamic_att_args.h"
#include "channel_dynamic_att_shared_data.h"
#include "channel_dynamic_att_com.h"
#include "channel_dynamic_att_com_protocol.h"
#include "channel_if.h"

static struct {
    double                      attenuation;
    size_t                      num_devices;
    ch_dynamic_protected_data_t att_matrix;
} ch_dynamic_att_prv;

static void channel_dynamic_att_read_att_matrix_entry(void *matrix_ptr, size_t matrix_size, void *read_data_ptr)
{
    ch_dynamic_att_com_protocol_t *read_peer_ptr;
    ch_dynamic_att_com_protocol_t *matrix_entry_ptr;

    read_peer_ptr              = (ch_dynamic_att_com_protocol_t *)read_data_ptr;
    matrix_entry_ptr           = &((ch_dynamic_att_com_protocol_t *)matrix_ptr)[read_peer_ptr->device];
    read_peer_ptr->attenuation = matrix_entry_ptr->attenuation;
}

static void channel_dynamic_att_write_att_matrix_entry(void *matrix_ptr, size_t matrix_size, void *write_data_ptr)
{
    ch_dynamic_att_com_protocol_t *write_peer_ptr;
    ch_dynamic_att_com_protocol_t *matrix_entry_ptr;

    write_peer_ptr                = (ch_dynamic_att_com_protocol_t *)write_data_ptr;
    matrix_entry_ptr              = &((ch_dynamic_att_com_protocol_t *)matrix_ptr)[write_peer_ptr->device];
    matrix_entry_ptr->attenuation = write_peer_ptr->attenuation;
}

static void channel_dynamic_att_incoming_cb(void *data_ptr, size_t data_size)
{
    ch_dynamic_att_com_protocol_t *write_data_ptr;

    if (data_size == sizeof(ch_dynamic_att_com_protocol_t)) {
        write_data_ptr = (ch_dynamic_att_com_protocol_t *)data_ptr;
        if (write_data_ptr->device < ch_dynamic_att_prv.num_devices) {
            channel_dynamic_att_shared_data_access(&ch_dynamic_att_prv.att_matrix, channel_dynamic_att_write_att_matrix_entry, write_data_ptr);
        } else {
            bs_trace_warning_line("Received att data for device %u (must be from 0 to %u)\n",
                                  write_data_ptr->device,
                                  ch_dynamic_att_prv.num_devices - 1);
        }
    } else {
        bs_trace_warning_line("Received att data of size %u (must be %u)\n", data_size, sizeof(ch_dynamic_att_com_protocol_t));
    }
}

static void channel_dynamic_att_populate_attenuation_matrix(void)
{
    ch_dynamic_att_com_protocol_t *peer_data;

    for (size_t i = 0; i < ch_dynamic_att_prv.num_devices; i++) {
        peer_data              = &((ch_dynamic_att_com_protocol_t *)ch_dynamic_att_prv.att_matrix.data_ptr)[i];
        peer_data->device      = i;
        peer_data->attenuation = ch_dynamic_att_prv.attenuation;
    }
}

/*
 * Public API
 */

/**
 * Initialize the channel
 */
int channel_init(int argc, char *argv[], uint num_devices)
{
    ch_dynamic_att_args_t args;

    channel_dynamic_att_argparse(argc, argv, &args);

    ch_dynamic_att_prv.attenuation = args.default_attenuation;
    ch_dynamic_att_prv.num_devices = num_devices;

    channel_dynamic_att_shared_data_alloc(&ch_dynamic_att_prv.att_matrix, ch_dynamic_att_prv.num_devices * sizeof(ch_dynamic_att_com_protocol_t));
    channel_dynamic_att_populate_attenuation_matrix();
    channel_dynamic_att_com_open(args.sim_id, args.pipe_name, channel_dynamic_att_incoming_cb);

    return 0;
}

/**
 * Recalculate the fading and path loss of the channel in this current moment (<now>)
 * in between the N used paths and the receive path (<rxnbr>)
 *
 * inputs:
 *  tx_used    : array with n_devs elements, 0: that tx is not transmitting,
 *                                           1: that tx is transmitting,
 *               e.g. {0,1,1,0}: devices 1 and 2 are transmitting, device 0 and 3 are not.
 *  tx_list    : array with all transmissions status (the channel can check here the modulation type of the transmitter if necessary)
 *               (ignored in this channel)
 *  txnbr      : desired transmitter number (the channel will calculate the ISI only for the desired transmitter)
 *               (ignored in this channel)
 *  rxnbr      : device number which is receiving
 *               (ignored in this channel)
 *  now        : current time
 *               (ignored in this channel)
 *  att        : array with n_devs elements. The channel will overwrite the element i
 *               with the average attenuation from path i to rxnbr (in dBs)
 *               The caller allocates this array
 *  ISI_SNR    : The channel will return here an estimate of the SNR limit due to multipath
 *               caused ISI for the desired transmitter (in dBs)
 *               (This channel sets this value always to 100.0)
 *
 * Returns < 0 on error.
 * 0 otherwise
 */
int channel_calc(const uint *tx_used, tx_el_t *tx_list, uint txnbr, uint rxnbr, bs_time_t now, double *att, double *ISI_SNR)
{
    ch_dynamic_att_com_protocol_t peer_read_data = {0};

    for (peer_read_data.device = 0; peer_read_data.device < ch_dynamic_att_prv.num_devices; peer_read_data.device++) {
        if (tx_used[peer_read_data.device]) {
            channel_dynamic_att_shared_data_access(&ch_dynamic_att_prv.att_matrix, channel_dynamic_att_read_att_matrix_entry, &peer_read_data);
            att[peer_read_data.device] = peer_read_data.attenuation;
/*
      if (peer_read_data.device==1)
        bs_trace_raw(1, "att=%lf\n", peer_read_data.attenuation);
 */
        }
    }
    *ISI_SNR = 100;

    return 0;
}

/**
 * Clean up: Free the memory the channel may have allocate
 * close any file descriptors etc.
 * (the simulation has ended)
 */
void channel_delete()
{
    channel_dynamic_att_com_close();
    channel_dynamic_att_shared_data_free(&ch_dynamic_att_prv.att_matrix);
}
