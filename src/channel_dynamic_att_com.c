/*
 * Copyright 2023 Oticon A/S
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
#include <pthread.h>
#include <fcntl.h>
#include "bs_oswrap.h"
#include "bs_types.h"
#include "bs_tracing.h"
#include "bs_pc_base.h"
#include "channel_dynamic_att_com.h"
#include "channel_dynamic_att_com_protocol.h"

extern char *pb_com_path;

typedef struct {
    ch_dynamic_att_com_cb_t incoming_cb;
} ch_dynamic_att_com_thread_arg_t;

static struct {
    char     *pipe_full_path;
    int       write_pipe_handle;
    pthread_t tid;
} ch_dynamic_att_com_prv = {0};

static void channel_dynamic_att_com_prepare(char *sim_id, char *pipe_name)
{
    int folder_length = 0;

    if ((folder_length = pb_create_com_folder(sim_id)) <= 0) {
        bs_trace_error("Cannot create folder %s for communication pipe\n", sim_id);
    }

    ch_dynamic_att_com_prv.pipe_full_path = bs_calloc(folder_length + strlen(pipe_name) + 2, sizeof(char));
    sprintf(ch_dynamic_att_com_prv.pipe_full_path, "%s/%s", pb_com_path, pipe_name);
}

static void *channel_dynamic_att_com_worker(void *args_ptr)
{
    int                             read_pipe_handle;
    ssize_t                         bytes_read;
    ch_dynamic_att_com_protocol_t   buffer;
    ch_dynamic_att_com_thread_arg_t thread_args = *(ch_dynamic_att_com_thread_arg_t *)args_ptr;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if ((read_pipe_handle = open(ch_dynamic_att_com_prv.pipe_full_path, O_RDONLY)) != -1) {
        bs_trace_raw(8, "channel_dynamic_att ready for communication\n");
        do {
            bytes_read = read(read_pipe_handle, &buffer, sizeof(buffer));
            if (bytes_read == sizeof(buffer)) {
                if (buffer.device == DYNAMIC_ATT_PROTOCOL_TERMINATE_MSG) {
                    break;
                }
                bs_trace_raw(8, "channel_dynamic_att received new attenuation\n");
                thread_args.incoming_cb(&buffer, bytes_read);
            }
        } while (bytes_read == sizeof(buffer));
        if (bytes_read != sizeof(buffer)) {
            bs_trace_warning_line("Received malformed attenuation data, length = %u. Terminating receiver.\n", bytes_read);
        }
    } else {
        bs_trace_error("Failed opening pipe at location %s: %d\n", ch_dynamic_att_com_prv.pipe_full_path, read_pipe_handle);
    }
    if (read_pipe_handle) {
        close(read_pipe_handle);
    }
    bs_trace_raw(8, "channel_dynamic_att closing communication\n");
    pthread_exit(NULL);
}

void channel_dynamic_att_com_open(char *sim_id, char *pipe_name, ch_dynamic_att_com_cb_t incoming_cb)
{
    ch_dynamic_att_com_thread_arg_t thread_args = {
        .incoming_cb = incoming_cb
    };

    channel_dynamic_att_com_prepare(sim_id, pipe_name);
    if (pb_create_fifo_if_not_there(ch_dynamic_att_com_prv.pipe_full_path) != 0) {
        bs_trace_error("Failed creating pipe at location %s\n", ch_dynamic_att_com_prv.pipe_full_path);
    }

    if (pthread_create(&ch_dynamic_att_com_prv.tid, NULL, channel_dynamic_att_com_worker, (void *)&thread_args) != 0) {
        bs_trace_error("Failed creating communications worker thread\n");
    }
    // Create a write end for the attenuation pipe in order to shut down gracefully
    if ((ch_dynamic_att_com_prv.write_pipe_handle = open(ch_dynamic_att_com_prv.pipe_full_path, O_WRONLY)) == -1) {
        bs_trace_error("Failed opening pipe writer\n");
    }
}

void channel_dynamic_att_com_close(void)
{
    ch_dynamic_att_com_protocol_t buffer = {
        .device      = DYNAMIC_ATT_PROTOCOL_TERMINATE_MSG,
        .attenuation = 0
    };

    if (ch_dynamic_att_com_prv.tid) {
        if (ch_dynamic_att_com_prv.write_pipe_handle) {
            if (write(ch_dynamic_att_com_prv.write_pipe_handle, &buffer, sizeof(buffer)) != sizeof(buffer)) {
                bs_trace_raw(3, "Could not terminate pipe. Process will be terminated by caller\n");
                ch_dynamic_att_com_prv.write_pipe_handle = 0;
            }
            bs_trace_raw(8, "channel_dynamic_att signaled closing of communication\n");
        }
        pthread_join(ch_dynamic_att_com_prv.tid, NULL);
        bs_trace_raw(8, "channel_dynamic_att communication closed\n");
    }
    if (ch_dynamic_att_com_prv.pipe_full_path) {
        if (ch_dynamic_att_com_prv.write_pipe_handle) {
            close(ch_dynamic_att_com_prv.write_pipe_handle);
        }
        remove(ch_dynamic_att_com_prv.pipe_full_path);
        free(ch_dynamic_att_com_prv.pipe_full_path);
        ch_dynamic_att_com_prv.pipe_full_path = NULL;
    }
}
