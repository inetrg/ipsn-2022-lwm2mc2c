/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "msg.h"
#include "shell.h"

#include "net/netif.h"

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

    puts("app;baseline_dtls");
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

    if (IS_ACTIVE(CONFIG_USE_CUSTOM_DATA_RATE)) {
        printf("app;dr;%d\n",CONFIG_DATA_RATE);

        netif_t *interface = netif_iter(NULL);
        if (!interface) {
            puts("app;NO_INTERFACE");
            return -1;
        }

        uint8_t dr = CONFIG_DATA_RATE;
        if (netif_set_opt(interface, NETOPT_OQPSK_RATE, 0, &dr, sizeof(uint8_t)) < 0) {
            puts("app;dr;err");
            return -1;
        }
    }

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(my_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
