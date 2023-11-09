/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _CHANNEL_DYNAMIC_ATT_CLIENT_H
#define _CHANNEL_DYNAMIC_ATT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dynamic attenuation client
 *
 * This module provide the means of dynamically manipulating the attenuation between this device and others.
 *
 * In order to use this, the test must use the dynamic_att channel and an approximated modem such as CSEM, eg.
 *     -channel=dynamic_att -defmodem=CSEMv1 -argschannel -s=${sim_id}
 *
 * The caller must both initialize by calling @ref channel_dynamic_att_client_open and clean up by @ref calling gm_lib_dynamic_att_close.
 *
 * Commands are guaranteed to be sent unfragmented as multiple client may be connected.
 */

/**
 * @brief Initialize dynamic attenuation feature
 *
 * This will open a write-only fifo to the ext_2G4_channel_dynamic_att channel for sending commands
 *
 * @param fifo_name Optional name of the fifo. This must correspond to the -fifo_name parameter passed to ext_2G4_channel_dynamic_att.
 *                  Pass NULL here to use the default name.
 * @return True on success
 */
bool channel_dynamic_att_client_open(char *fifo_name);

/**
 *  @brief Close the fifo opened by @ref channel_dynamic_att_client_open
 */
void channel_dynamic_att_client_close(void);

/**
 * @brief Reset all attenuations to default.
 *
 * @return True if sending command is successful
 */
bool channel_dynamic_att_client_reset(void);

/**
 * @brief Write attenuation for this device.
 *
 * This will set the attenuation between this device and all others.
 *
 * @param attenuation The attenuation in dBm for the next packet until changed.
 * @return True if sending command is successful
 */
bool channel_dynamic_att_client_set_attenuation_all(double attentuation);

/**
 * @brief Write attenuation between this device and a peer.
 *
 * This will set both the Rx and Tx attenuation between this device and another.
 *
 * @param peer_device The device number of the peer.
 * @param rx_attenuation The attenuation in dBm for incoming packets to this device.
 * @param tx_attenuation The attenuation in dBm for outgoing packets from this device.
 * @return True if sending command is successful
 */
bool channel_dynamic_att_client_set_attenuation_one(unsigned short peer_device, double rx_attentuation, double tx_attentuation);

#ifdef __cplusplus
}
#endif

#endif /* _CHANNEL_DYNAMIC_ATT_CLIENT_H */
