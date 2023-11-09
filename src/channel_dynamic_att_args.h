/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_CHANNEL_DYNAMIC_ATT_ARGS_H
#define BS_CHANNEL_DYNAMIC_ATT_ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char  *sim_id;
    double default_attenuation;
    char  *pipe_name;
} ch_dynamic_att_args_t;

void channel_dynamic_att_argparse(int argc, char *argv[], ch_dynamic_att_args_t *args);

#ifdef __cplusplus
}
#endif

#endif // BS_CHANNEL_DYNAMIC_ATT_ARGS_H
