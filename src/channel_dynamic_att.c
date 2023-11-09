/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "channel_dynamic_att_args.h"
#include "channel_dynamic_att_com.h"
#include "channel_if.h"

static struct {
    double  default_attenuation;
    uint    num_devices;
    double *attenuation_matrix;
} ch_dynamic_att_prv = {0};

/*
 * The attenuation matrix is organized as rows for the Tx device and columns for the Rx device.
 * To find the attenuation between two devices select the Tx device row and then the Rx column.
 * Note that attenuation can be different for each direction.
 *
 * This is an example of the attenuation matrix when attenuation is set to 100 dBm between
 * device 2 and all the others:
 *
 *    \ Rx 0   1   2   3
 *   Tx +----------------
 *    0 |  -   d  100  d
 *    1 |  d   -  100  d
 *    2 | 100 100  -  100
 *    3 |  d   d  100  -
 *
 * This is an example of the attenuation matrix when Rx attenuation to (this) device 0 and 3 is
 * set to 99 dBm and Tx attenuation is set to 77 dBm:
 *
 *    \ Rx 0   1   2   3
 *   Tx +----------------
 *    0 |  -   d   d   99
 *    1 |  d   -   d   d
 *    2 |  d   d   -   d
 *    3 |  77  d   d   -
 *
 *  Legend: 'd' is the default attenuation.
 */

static void channel_dynamic_att_reset_matrix(void)
{
    if (ch_dynamic_att_prv.attenuation_matrix) {
        for (uint device = 0; device < ch_dynamic_att_prv.num_devices*ch_dynamic_att_prv.num_devices; device++) {
            ch_dynamic_att_prv.attenuation_matrix[device] = ch_dynamic_att_prv.default_attenuation;
        }
    }
}

static double *channel_dynamic_att_get_att_ptr(unsigned short row, unsigned short col)
{
    return ch_dynamic_att_prv.attenuation_matrix + row*ch_dynamic_att_prv.num_devices + col;
}

static void channel_dynamic_att_set_all_for_dev(ch_dynamic_att_com_protocol_set_att_all_t *payload)
{
    if (payload->device >= ch_dynamic_att_prv.num_devices) {
        bs_trace_error_line("Error: device parameter is out of bounds\n");
    }

    for (uint dev = 0U; dev < ch_dynamic_att_prv.num_devices; dev++) {
        /* Update tx row */
        *channel_dynamic_att_get_att_ptr(payload->device, dev) = payload->attenuation;
        /* Update rx column */
        *channel_dynamic_att_get_att_ptr(dev, payload->device) = payload->attenuation;
    }
}

static void channel_dynamic_att_set_one_for_dev(ch_dynamic_att_com_protocol_set_att_one_t *payload)
{
    if (payload->device >= ch_dynamic_att_prv.num_devices) {
        bs_trace_error_line("Error: device parameter is out of bounds: %u\n", payload->device);
    }
    if (payload->peer_device >= ch_dynamic_att_prv.num_devices || payload->device == payload->peer_device) {
        bs_trace_error_line("Error: peer_device parameter is out of bounds: %u\n", payload->peer_device);
    }

    /* Update tx attenuation */
    *channel_dynamic_att_get_att_ptr(payload->device, payload->peer_device) = payload->attenuation_tx;
    /* Update rx attenuation */
    *channel_dynamic_att_get_att_ptr(payload->peer_device, payload->device) = payload->attenuation_tx;
}

static void channel_dynamic_att_check(void)
{
    ch_dynamic_att_com_protocol_packet_t packet = {0};

    if (channel_dynamic_att_com_read_packet(&packet)) {
        switch (packet.header.command) {
            case DYNAMIC_ATT_PROTOCOL_CMD_RESET:
                channel_dynamic_att_reset_matrix();
                bs_trace_raw(8, "All attenuation settings was reset to default (%lu)\n", ch_dynamic_att_prv.default_attenuation);
                break;
            case DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ALL:
                channel_dynamic_att_set_all_for_dev(&packet.payload.set_att_all_payload);
                bs_trace_raw(8, "Updated attenuation for all connections with device %u\n", packet.payload.set_att_all_payload.device);
                break;
            case DYNAMIC_ATT_PROTOCOL_CMD_SET_ATT_ONE:
                channel_dynamic_att_set_one_for_dev(&packet.payload.set_att_one_payload);
                bs_trace_raw(8, "Updated attenuation for connections between device %u and %u\n", packet.payload.set_att_one_payload.device, packet.payload.set_att_one_payload.peer_device);
                break;
            default:
                bs_trace_warning_line_time("Received unknown command %u\n", packet.payload.set_att_all_payload.device);
        }
    }
}

/*
 * Public API
 */

/**
 * @brief Initialize this channel
 *
 * Allocate attenuation matrix and open fifo for receiving commands.
 */
int channel_init(int argc, char *argv[], uint num_devices)
{
    ch_dynamic_att_args_t args;

    channel_dynamic_att_argparse(argc, argv, &args);

    ch_dynamic_att_prv.default_attenuation = args.default_attenuation;
    ch_dynamic_att_prv.num_devices = num_devices;

    ch_dynamic_att_prv.attenuation_matrix = bs_calloc(num_devices*num_devices, sizeof(double));
    if (!ch_dynamic_att_prv.attenuation_matrix) {
        bs_trace_error("Error allocating memory for attenuation matrix");
    }
    channel_dynamic_att_reset_matrix();

    channel_dynamic_att_com_open(args.sim_id, args.fifo_name);

    return 0;
}

/**
 * @brief Apply attenuation if any
 *
 * Recalculate the path loss of the channel in this current moment (<now>)
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
 *               used for looking up the attenuation between the two devices
 *  now        : current time
 *               (ignored in this channel)
 *  att        : array with n_devs elements. The channel will overwrite the element i
 *               with the average attenuation from path i to rxnbr (in dBm)
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
    channel_dynamic_att_check();

    for (uint device = 0U; device < ch_dynamic_att_prv.num_devices; device++) {
        if (tx_used[device]) {
            att[device] = *channel_dynamic_att_get_att_ptr(device, rxnbr);
        }
    }
    *ISI_SNR = 100;

    return 0;
}

/**
 * @brief Clean up
 *
 * Free the memory the channel may have allocated and close fifo.
 * (the simulation has ended)
 */
void channel_delete()
{
    channel_dynamic_att_com_close();

    if (ch_dynamic_att_prv.attenuation_matrix) {
        free(ch_dynamic_att_prv.attenuation_matrix);
    }
}
