/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_CHANNEL_DYNAMIC_ATT_COM_H
#define BS_CHANNEL_DYNAMIC_ATT_COM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ch_dynamic_att_com_cb_t)(void *data_ptr, size_t data_size);

void channel_dynamic_att_com_open(char *sim_id, char *pipe_name, ch_dynamic_att_com_cb_t incoming_cb);
void channel_dynamic_att_com_close(void);

#ifdef __cplusplus
}
#endif

#endif // BS_CHANNEL_DYNAMIC_ATT_COM_H
