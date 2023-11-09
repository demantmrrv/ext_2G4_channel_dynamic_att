/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _CHANNEL_DYNAMIC_ATT_ARGS_H
#define _CHANNEL_DYNAMIC_ATT_ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char   *sim_id;
    double  default_attenuation;
    char   *fifo_name;
} ch_dynamic_att_args_t;

/**
 * @brief Parse arguments to this channel
 *
 * Arguments supported are:
 *   's' or 'sim_id',        mandatory: The current BSIM ID for the simulation
 *   'att' or 'attenuation', optional : The default attenuation between all devices
 *   'fn' or 'fifo_name',    optional : The name of the fifo used for receiving commands from client
*/
void channel_dynamic_att_argparse(int argc, char *argv[], ch_dynamic_att_args_t *args);

#ifdef __cplusplus
}
#endif

#endif // _CHANNEL_DYNAMIC_ATT_ARGS_H
