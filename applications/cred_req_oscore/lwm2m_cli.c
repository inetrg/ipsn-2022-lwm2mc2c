/*
 * Copyright (C) 2019 HAW Hamburg
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
 * @brief       Wakaama LwM2M Client CLI support
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_platform.h"
#include "objects/security.h"
#include "objects/device.h"
#include "objects/light_control.h"
#include "objects/access_control.h"
#include "objects/server.h"
#include "objects/common.h"
#include "objects/oscore.h"

#include "fmt.h"
#include "net/credman.h"
#include "dtls_creds.h"
#include "../common.h"

#define EVENT_AUTH_MARK         "auth"
#define EVENT_AUTH_BEGIN(id)    EVENT_BEGIN(EVENT_AUTH_MARK, id)
#define EVENT_AUTH_END(id)      EVENT_END(EVENT_AUTH_MARK, id)
#define EVENT_AUTH_ERR(id)      EVENT_END(EVENT_AUTH_MARK, id)

#define EVENT_READ_MARK         "read"
#define EVENT_READ_BEGIN(id)    EVENT_BEGIN(EVENT_READ_MARK, id)
#define EVENT_READ_END(id)      EVENT_END(EVENT_READ_MARK, id)
#define EVENT_READ_ERR(id)      EVENT_END(EVENT_READ_MARK, id)

#define LED_COLOR   "000"
#define LED_APP_TYPE "LED 0"
#define OBJ_COUNT (9)

uint8_t connected = 0;
uint8_t obj_count = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
lwm2m_client_data_t client_data;

unsigned auth_counter = 0;

static lwm2m_uri_t _resource_uri = {
    .objectId = CONFIG_RESOURCE_OBJ_ID,
    .instanceId = CONFIG_RESOURCE_INST_ID,
    .resourceId = CONFIG_RESOURCE_ID,
    .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
};

int lwm2m_cli_init(void)
{
    lwm2m_object_t *obj = NULL;

    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    lwm2m_obj_security_args_t sec_args = {
        .server_id = CONFIG_LWM2M_SERVER_SHORT_ID,
        .server_uri = CONFIG_LWM2M_SERVER_URI,
        .security_mode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
        .cred = &credential,
        .is_bootstrap = IS_ACTIVE(LWM2M_SERVER_IS_BOOTSTRAP), /* set to true when using Bootstrap server */
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = LWM2M_MAX_ID,
    };

    obj = lwm2m_object_security_get();
    int res = lwm2m_object_security_instance_create(obj, 0, &sec_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the security object");
        return -1;
    }

    obj = lwm2m_object_server_get();
    lwm2m_obj_server_args_t server_args = {
        .short_id = CONFIG_LWM2M_SERVER_SHORT_ID, /* must match the one use in the security */
        .binding = BINDING_U,
        .lifetime = 480, /* 8 minutes */
        .max_period = 240,
        .min_period = 180,
        .notification_storing = false,
        .disable_timeout = 3600 /* one hour */
    };

    res = lwm2m_object_server_instance_create(obj, 0, &server_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the server object");
        return -1;
    }

    /* device object has a single instance. All the information for now is defined at
     * compile-time */
    obj_list[obj_count++] = lwm2m_object_device_get();

    /* configure access to light control */
    obj_list[obj_count++] = lwm2m_object_access_control_get();

    obj = lwm2m_object_client_get();
#if IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)
        /* the requester has the host information beforehand */
        lwm2m_obj_client_args_t client_args = {
            .short_id = CONFIG_CLIENT_HOST_SHORT_ID,
            .endpoint = CONFIG_CLIENT_HOST_ENDPOINT,
            .binding = BINDING_U,
            .lifetime = 120,
            .max_period = 120,
            .min_period = 60,
            .notification_storing = false,
            .disable_timeout = 3600
        };
        lwm2m_object_client_instance_create(obj, 0, &client_args);
#endif
    obj_list[obj_count++] = obj;

    obj = lwm2m_object_client_security_get();
#if IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)
        lwm2m_obj_client_security_args_t client_sec_args = {
            .server_id = CONFIG_CLIENT_HOST_SHORT_ID,
            .server_uri = CONFIG_CLIENT_HOST_URI,
            .security_mode = LWM2M_SECURITY_MODE_NONE,
            .cred = NULL,
            .is_bootstrap = false,
            .client_hold_off_time = 5,
            .bootstrap_account_timeout = 0,
            .oscore_object_inst_id = LWM2M_MAX_ID,
        };
        lwm2m_object_client_security_instance_create(obj, 0, &client_sec_args);
#endif
    obj_list[obj_count++] = obj;

    obj_list[obj_count++] = lwm2m_object_client_access_control_get();
    obj_list[obj_count++] = lwm2m_object_oscore_get();

    obj = lwm2m_object_light_control_get();
    lwm2m_obj_light_control_args_t light_args = {
        .cb = NULL,
        .cb_arg = NULL,
        .color = LED_COLOR,
        .color_len = sizeof(LED_COLOR) - 1,
        .app_type = LED_APP_TYPE,
        .app_type_len = sizeof(LED_APP_TYPE) - 1
    };

    res = lwm2m_object_light_control_instance_create(obj, 0, &light_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating light control");
        return -1;
    }

    /* check that the object number is within boundaries */
    if (obj_count > OBJ_COUNT) {
        puts("Too many objects created!! Increase OBJ_COUNT");
        return -1;
    }

    return 0;
}

void _read_cb(uint16_t client_id, lwm2m_uri_t *uri, int status, lwm2m_media_type_t format,
              uint8_t *data, int data_len, void *user_data)
{
    (void) client_id;
    (void) uri;
    (void) format;
    (void) data_len;
    (void) user_data;
    (void) status;

    if (data) {
        printf(EVENT_READ_END("%d") "\n", auth_counter);
    }
    else {
        printf(EVENT_READ_ERR("%d") "\n", auth_counter);
    }
}

int lwm2m_cli_cmd_read(void)
{
    printf(EVENT_READ_BEGIN("%d") "\n", auth_counter);
    int res = lwm2m_client_read(&client_data, 0, &_resource_uri, _read_cb);
    if (res != COAP_231_CONTINUE) {
        printf("Error reading from client\n");
        return 1;
    }

    return 0;
}

void _auth_cb(uint16_t short_server_id, uint8_t response_code, void *user_data)
{
    (void) user_data;
    (void) short_server_id;

    if (response_code == COAP_201_CREATED) {
        printf(EVENT_AUTH_END("%d") "\n", auth_counter);
        lwm2m_cli_cmd_read();
    }
    else {
        printf(EVENT_AUTH_ERR("%d") "\n", auth_counter);
    }
}

int lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        /* run the LwM2M client */
        if (!connected && lwm2m_client_run(&client_data, obj_list, ARRAY_SIZE(obj_list))) {
            connected = 1;
        }
        return 0;
    }

    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)) {
        if (!strcmp(argv[1], "read")) {
            return lwm2m_cli_cmd_read();
        }

        if (!strcmp(argv[1], "auth")) {

            if (argc == 3) {
                auth_counter = atoi(argv[2]);
            }

            int server_id = CONFIG_LWM2M_SERVER_SHORT_ID;
            char client_ep[] = "client_02";

            lwm2m_auth_request_t req;
            req.access = 1; /* read access */
            req.uri.objectId = _resource_uri.objectId;
            req.uri.instanceId = _resource_uri.instanceId;
            req.uri.flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID;

            printf(EVENT_AUTH_BEGIN("%d") "\n", auth_counter);

            int res = lwm2m_request_cred_and_auth(&client_data, server_id, client_ep,
                                                  strlen(client_ep), &req, 1, true, _auth_cb);
            if (res != COAP_231_CONTINUE) {
                printf("Error observing client's resource\n");
                return 1;
            }

            return 0;
        }
    }

help_error:
    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)) {
        printf("usage: %s <start|auth|read>\n", argv[0]);
        printf("- start: will trigger client initialization and registration to server\n");
        printf("- auth: will request credentials an access rights to /%d/%d in the host client\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID);
        printf("- read: will attempt to read the resource /%d/%d/%d in the host client\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID, CONFIG_RESOURCE_ID);
    }
    else {
        printf("usage: %s <start>\n", argv[0]);
        printf("- start: will trigger client initialization and registration to server\n");
    }

    return 1;
}
