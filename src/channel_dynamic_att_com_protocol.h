/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H
#define BS_CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H
#include <limits.h>
#include "bs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DYNAMIC_ATT_PROTOCOL_TERMINATE_MSG UINT_MAX

typedef struct {
    uint   device;
    double attenuation;
} __attribute__((packed)) ch_dynamic_att_com_protocol_t;

#ifdef __cplusplus
}
#endif

#endif // BS_CHANNEL_DYNAMIC_ATT_COM_PROTOCOL_H
