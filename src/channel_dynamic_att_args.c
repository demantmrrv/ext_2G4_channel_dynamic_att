/*
 * Copyright 2024 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_cmd_line.h"
#include "bs_tracing.h"
#include "channel_dynamic_att_args.h"
#include "channel_dynamic_att_defaults.h"

static char library_name[] = "Dynamic attenautor 2G4 channel";

void component_print_post_help()
{
    fprintf(stdout, "This is a non realistic channel model.\n"
            "It models NxN independent paths each with dynamically configurable attenuation.\n"
            "By default the overall attenuation is %d dBm. However, it can be changed\n"
            "dynamically and independently for each connection and its directions.\n",
            DYNAMIC_ATT_DEFAULT
            );
}

/**
 * Check the arguments provided in the command line: set args based on it
 * or defaults, and check they are correct
 */
void channel_dynamic_att_argparse(int argc, char *argv[], ch_dynamic_att_args_t *args)
{
    bs_args_struct_t args_struct[] = {
        /*manual,mandatory,switch,option,   name ,     type,   destination,               callback,      , description*/
        {false, true,  false,  "s",     "sim_id",        's',        (void *)&args->sim_id,                    NULL,
         "Sim id. Must be used to allow parallel channels."               },
        {false, false, false,  "att",   "attenuation",   'f',        (void *)&args->default_attenuation,       NULL,
         "Initial attenuation in dB, used in all NxN paths until changed."},
        {false, false, false,  "fn",    "fifo_name",     's',        (void *)&args->fifo_name,                 NULL,
         "Name of pipe for communication."                                },
        ARG_TABLE_ENDMARKER
    };

    args->default_attenuation = DYNAMIC_ATT_DEFAULT;
    args->fifo_name           = DYNAMIC_ATT_DEFAULT_FIFO_NAME;

    bs_args_override_exe_name(library_name);
    bs_args_set_trace_prefix("channel: (dynamic_att) ");
    bs_args_parse_all_cmd_line(argc, argv, args_struct);


    if ((args->default_attenuation < DYNAMIC_ATT_MIN) || (args->default_attenuation > DYNAMIC_ATT_MAX)) {
        bs_trace_error("channel: cmdarg: attenuation must be be between %lf dBm and %lf dBm (is %lf)\n",
                       DYNAMIC_ATT_MIN, DYNAMIC_ATT_MAX, args->default_attenuation);
    }
}
