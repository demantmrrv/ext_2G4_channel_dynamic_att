/*
 * Copyright 2023 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <pthread.h>
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "channel_dynamic_att_shared_data.h"

void channel_dynamic_att_shared_data_alloc(ch_dynamic_protected_data_t *shared_data_ptr, size_t data_size)
{
    if (!shared_data_ptr) {
        bs_trace_error("Cannot initialize empty data\n");
    }

    shared_data_ptr->data_ptr  = bs_calloc(1, data_size);
    shared_data_ptr->data_size = data_size;
    if (pthread_mutex_init(&shared_data_ptr->lock, NULL) != 0) {
        bs_trace_error("Mutex initialization failed\n");
    }
}

void channel_dynamic_att_shared_data_access(ch_dynamic_protected_data_t *shared_data_ptr,
                                            ch_dynamic_shared_data_access_cb_t access_ptr,
                                            void *args_ptr)
{
    if (!shared_data_ptr) {
        bs_trace_error("Cannot access empty data\n");
    }

    pthread_mutex_lock(&shared_data_ptr->lock);

    access_ptr(shared_data_ptr->data_ptr, shared_data_ptr->data_size, args_ptr);

    pthread_mutex_unlock(&shared_data_ptr->lock);
}

void channel_dynamic_att_shared_data_free(ch_dynamic_protected_data_t *shared_data_ptr)
{
    if (!shared_data_ptr) {
        bs_trace_error("Cannot initialize empty data\n");
    }

    if (shared_data_ptr->data_ptr) {
        free(shared_data_ptr->data_ptr);
        shared_data_ptr->data_ptr  = NULL;
        shared_data_ptr->data_size = 0;

        pthread_mutex_destroy(&shared_data_ptr->lock);
    }
}
