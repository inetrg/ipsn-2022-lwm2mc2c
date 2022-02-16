/*
 * Copyright (C) 2018 Beduino Master Projekt - University of Bremen
 *               2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for Eclipse Wakaama LwM2M Client
 *
 * @author      Christian Manal <manal@uni-bremen.de>
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "msg.h"
#include "shell.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

extern void setup_routing(void);
extern int lwm2m_cli_init(void);
extern int lwm2m_cli_cmd(int argc, char **argv);
static const shell_command_t my_commands[] = {
    { "lwm2m", "Start LwM2M client", lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{

    puts("app;cred_req_dtls");
    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)) {
        puts("node;requester");
    }
    else {
        puts("node;host");
    }

    /* initiates LwM2M client */
    int res = lwm2m_cli_init();
    if (res) {
        puts("app;err");
        return -1;
    }

    setup_routing();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(my_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
