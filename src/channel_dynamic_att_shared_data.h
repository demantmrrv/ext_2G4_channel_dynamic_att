/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_CHANNEL_DYNAMIC_ATT_SHARED_DATA_H
#define BS_CHANNEL_DYNAMIC_ATT_SHARED_DATA_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void           *data_ptr;
    size_t          data_size;
    pthread_mutex_t lock;
} ch_dynamic_protected_data_t;

typedef void (*ch_dynamic_shared_data_access_cb_t)(void *data_ptr, size_t data_size, void *args_ptr);

void channel_dynamic_att_shared_data_alloc(ch_dynamic_protected_data_t *shared_data_ptr, size_t data_size);
void channel_dynamic_att_shared_data_access(ch_dynamic_protected_data_t *shared_data_ptr,
                                            ch_dynamic_shared_data_access_cb_t access_ptr,
                                            void *args_ptr);
void channel_dynamic_att_shared_data_free(ch_dynamic_protected_data_t *shared_data_ptr);

#ifdef __cplusplus
}
#endif

#endif // BS_CHANNEL_DYNAMIC_ATT_SHARED_DATA_H
