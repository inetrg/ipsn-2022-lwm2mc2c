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
#include "objects/oscore.h"
#include "objects/common.h"

#if IS_USED(MODULE_LEDS_SEQUENCE)
#include "leds_sequence.h"
#endif

#include "xtimer.h"
#include "fmt.h"
#include "dtls_creds.h"
#include "oscore_creds.h"
#include "../common.h"

#define EVENT_NOTIFICATION_MARK         "not"
#define EVENT_NOTIFICATION_BEGIN(id)    EVENT_BEGIN(EVENT_NOTIFICATION_MARK, id)
#define EVENT_NOTIFICATION_END(id)      EVENT_END(EVENT_NOTIFICATION_MARK, id)


#if IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)
# define OBJ_COUNT (8)
#else
# define OBJ_COUNT (9)
#endif

uint8_t connected = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
lwm2m_client_data_t client_data;

static lwm2m_uri_t _resource_uri = {
    .objectId = CONFIG_RESOURCE_OBJ_ID,
    .instanceId = CONFIG_RESOURCE_INST_ID,
    .resourceId = CONFIG_RESOURCE_ID,
    .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
};

int lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);
    lwm2m_object_t *obj = NULL;
    uint8_t obj_count = 0;

    /* add security instance for the server */
    lwm2m_obj_security_args_t sec_args = {
        .server_id = CONFIG_LWM2M_SERVER_SHORT_ID,
        .server_uri = CONFIG_LWM2M_SERVER_URI,
        .security_mode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
        .cred = &credential,
        .is_bootstrap = IS_ACTIVE(LWM2M_SERVER_IS_BOOTSTRAP), /* set to true when using Bootstrap server */
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = LWM2M_MAX_ID
    };

    obj = lwm2m_object_security_get();
    int res = lwm2m_object_security_instance_create(obj, 0, &sec_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the security object");
        return -1;
    }

    /* add server instance */
    obj = lwm2m_object_server_get();
    lwm2m_obj_server_args_t server_args = {
        .short_id = CONFIG_LWM2M_SERVER_SHORT_ID, /* must match the one use in the security */
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
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

    /* configure server access to light control instance, by creating an access control instance */
    obj = lwm2m_object_access_control_get();
    uint16_t acc_ctrl_inst = 0;
    lwm2m_obj_access_control_args_t acc_args = {
        .owner = CONFIG_LWM2M_SERVER_SHORT_ID,
        .obj_id = LWM2M_LIGHT_CONTROL_OBJECT_ID,
        .obj_inst_id = 0
    };

    res = lwm2m_object_access_control_instance_create(obj, acc_ctrl_inst, &acc_args);
    if (res < 0) {
        puts("Error instantiating access control");
        return -1;
    }

    /* create access control list instance: by default all servers can read and write */
    lwm2m_obj_access_control_acl_args_t acl_args = {
        .access = LWM2M_ACCESS_CONTROL_READ | LWM2M_ACCESS_CONTROL_WRITE,
        .res_inst_id = 0
    };
    res = lwm2m_object_access_control_add(obj, acc_ctrl_inst, &acl_args);
    obj_list[obj_count++] = obj;

    /* create OSCORE instance for client */
    obj = lwm2m_object_oscore_get();
    lwm2m_obj_oscore_args_t oscore_args = {
        .aead_algorithm = AEAD_ALG_AES_CCM_16_64_128,
        .hmac_algorithm = HMAC_ALG_SHA_256,
        .id_ctx = NULL,
        .id_ctx_len = 0,
        .master_salt = master_salt,
        .master_salt_len = sizeof(master_salt) - 1,
        .master_secret = master_secret,
        .master_secret_len = sizeof(master_secret) - 1,
        .recipient_id = recipient_id,
        .recipient_id_len = sizeof(recipient_id) - 1,
        .sender_id = sender_id,
        .sender_id_len = sizeof(sender_id) - 1,
    };
    res = lwm2m_object_oscore_instance_create(obj, OSCORE_OBJECT_INSTANCE_ID, &oscore_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the OSCORE object");
        return -1;
    }

    /* add client security object instance, we are using OSCORE */
    obj = lwm2m_object_client_security_get();
    lwm2m_obj_client_security_args_t client_sec_args = {
        .server_id = CONFIG_LWM2M_CLIENT_SHORT_ID,
        .server_uri = CONFIG_LWM2M_CLIENT_URI,
        .security_mode = LWM2M_SECURITY_MODE_NONE,
        .cred = NULL,
        .is_bootstrap = false,
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = OSCORE_OBJECT_INSTANCE_ID
    };
    res = lwm2m_object_client_security_instance_create(obj, 0, &client_sec_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating client security object");
        return -1;
    }

    /* add client object instance */
    obj = lwm2m_object_client_get();
    lwm2m_obj_client_args_t client_args = {
        .short_id = CONFIG_LWM2M_CLIENT_SHORT_ID,
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
        .notification_storing = false,
        .disable_timeout = 3600, /* one hour */
        .endpoint = CONFIG_LWM2M_CLIENT_ENDPOINT
    };
    res = lwm2m_object_client_instance_create(obj, 0, &client_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating client object");
        return -1;
    }

    obj = lwm2m_object_light_control_get();
    lwm2m_obj_light_control_args_t light_args = {
        .cb = NULL,
        .cb_arg = NULL,
        .color = CONFIG_LED_COLOR,
        .color_len = sizeof(CONFIG_LED_COLOR) - 1,
        .app_type = CONFIG_LED_APP_TYPE,
        .app_type_len = sizeof(CONFIG_LED_APP_TYPE) - 1
    };

    res = lwm2m_object_light_control_instance_create(obj, 0, &light_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating light control");
        return -1;
    }

    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_HOST)) {
        /* the host needs to add access rights for the client to the light object */
        obj = lwm2m_object_client_access_control_get();
        acc_args.owner = CONFIG_LWM2M_SERVER_SHORT_ID;
        acc_args.obj_id = LWM2M_LIGHT_CONTROL_OBJECT_ID;
        acc_args.obj_inst_id = 0;
        res = lwm2m_object_access_control_instance_create(obj, 0, &acc_args);
        if (res < 0) {
            puts("Error instantiating client access control");
            return -1;
        }

        /* now add the proper ACL */
        acl_args.access = LWM2M_ACCESS_CONTROL_READ;
        acl_args.res_inst_id = CONFIG_LWM2M_CLIENT_SHORT_ID;
        res = lwm2m_object_access_control_add(obj, 0, &acl_args);
        obj_list[obj_count++] = obj;
        if (res < 0) {
            puts("Error instantiating ACL client access control");
            return -1;
        }
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
    (void) data;
    (void) status;
    (void) data_len;
    (void) user_data;

    if (data) {
        /* skip 3 positions to the string value */
        const unsigned skip = 3;
        if (!IS_ACTIVE(CONFIG_DISABLE_LOGGING)) {
            printf(EVENT_NOTIFICATION_END("%.*s") "\n", data_len - skip, &data[skip]);
        }
    }
    else {
        puts("err;");
    }
}

int lwm2m_cli_cmd_changed(void)
{
    static uint32_t counter = 0;
    static char buf[] = "00000";

    counter++;
    sprintf(buf, "%05" PRIu32, counter);

    /* the chosen resource should support to be written as string ! */
    int res = lwm2m_set_string(&client_data, &_resource_uri, buf, strlen(buf));
    assert(res >= 0);

    if (!IS_ACTIVE(CONFIG_DISABLE_LOGGING)) {
        printf(EVENT_NOTIFICATION_BEGIN("%s") "\n", buf);
    }

    lwm2m_resource_value_changed(client_data.lwm2m_ctx, &_resource_uri);
    lwm2m_client_force_step();
    return 0;
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
            int res = lwm2m_client_read(&client_data, 0, &_resource_uri, _read_cb);
            if (res != COAP_231_CONTINUE) {
                printf("Error reading from client\n");
                return 1;
            }

            return 0;
        }

        if (!strcmp(argv[1], "obs")) {
            int res = lwm2m_client_observe(&client_data, 0, &_resource_uri, _read_cb);
            if (res != COAP_231_CONTINUE) {
                printf("Error observing client's resource\n");
                return 1;
            }

            return 0;
        }
    }
    else { /* CONFIG_CLIENT_TYPE_HOST */
        /* mark a resource as changed, to trigger notifications if observed */
        if (!strcmp(argv[1], "changed")) {
            return lwm2m_cli_cmd_changed();
        }

        if (!strcmp(argv[1], "setup")) {
            uint32_t update_period_ms = atoi(argv[2]);
            uint32_t iterations = atoi(argv[3]);

#if IS_USED(MODULE_LEDS_SEQUENCE)
            leds_sequence_start();
#endif

            for (uint32_t i = 0; i < iterations; i++) {
                lwm2m_cli_cmd_changed();
                xtimer_msleep(update_period_ms);
            }

#if IS_USED(MODULE_LEDS_SEQUENCE)
            leds_sequence_end();
#endif
            return 0;
        }
    }


help_error:

    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)) {
        printf("usage: %s <start|read|obs>\n", argv[0]);
        printf("- start: will trigger client initialization and registration to server\n");
        printf("- read: will attempt to read the resource /%d/%d/%d in the host client\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID, CONFIG_RESOURCE_ID);
        printf("- obs: will attempt to observe the resource /%d/%d/%d in the host client\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID, CONFIG_RESOURCE_ID);
    }
    else {
        printf("usage: %s <start|changed>\n", argv[0]);
        printf("- start: will trigger client initialization and registration to server\n");
        printf("- changed: will mark the resource /%d/%d/%d as updated, and trigger notifications if needed\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID, CONFIG_RESOURCE_ID);
    }
    return 1;
}
