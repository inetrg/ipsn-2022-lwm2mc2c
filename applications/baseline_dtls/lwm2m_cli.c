/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
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

#if IS_USED(MODULE_LEDS_SEQUENCE)
#include "leds_sequence.h"
#endif

#include "xtimer.h"
#include "fmt.h"
#include "dtls_creds.h"
#include "../common.h"

#define EVENT_NOTIFICATION_MARK         "not"
#define EVENT_NOTIFICATION_BEGIN(id)    EVENT_BEGIN(EVENT_NOTIFICATION_MARK, id)
#define EVENT_NOTIFICATION_END(id)      EVENT_END(EVENT_NOTIFICATION_MARK, id)

# define OBJ_COUNT (4)

uint8_t connected = 0;
uint8_t obj_count = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
lwm2m_client_data_t client_data;

static lwm2m_uri_t _resource_uri = {
    .objectId = CONFIG_RESOURCE_OBJ_ID,
    .instanceId = CONFIG_RESOURCE_INST_ID,
    .resourceId = CONFIG_RESOURCE_ID,
    .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
};

void _led_cb(lwm2m_object_t *object, uint16_t instance_id, bool status, uint8_t dimmer,
             const char *color, size_t color_len, void *arg)
{
    (void)object;
    (void)instance_id;
    (void)arg;
    (void)status;
    (void)dimmer;

    // printf("val;%.*s\n", color_len, );
    if (!IS_ACTIVE(CONFIG_DISABLE_LOGGING)) {
        printf(EVENT_NOTIFICATION_END("%.*s") "\n", color_len, color);
    }
}

int lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);
    lwm2m_object_t *obj = NULL;

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

    printf("Credential Identity %s\n", (char*)credential.params.psk.id.s);

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
        .lifetime = 15000, /* two minutes */
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

    obj = lwm2m_object_light_control_get();
    lwm2m_obj_light_control_args_t light_args = {
        .cb = NULL,
        .cb_arg = NULL,
        .color = CONFIG_LED_COLOR,
        .color_len = sizeof(CONFIG_LED_COLOR) - 1,
        .app_type = CONFIG_LED_APP_TYPE,
        .app_type_len = sizeof(CONFIG_LED_APP_TYPE) - 1
    };

    if (IS_ACTIVE(CONFIG_CLIENT_TYPE_REQUESTER)) {
        light_args.cb = _led_cb;
    }

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

int lwm2m_cli_cmd_changed(void)
{
    static uint32_t counter = 0;
    static char buf[] = "00000";

    counter++;
    sprintf(buf, "%05" PRIu32, counter);

    /* the chosen resource should support to be written as string ! */
    int res = lwm2m_set_string(&client_data, &_resource_uri, buf, strlen(buf));
    assert(res >= 0);

    // printf("cha;%s\n",buf);
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
        if (!connected && lwm2m_client_run(&client_data, obj_list, obj_count)) {
            connected = 1;
        }
        return 0;
    }

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

help_error:

        printf("usage: %s <start|changed>\n", argv[0]);
        printf("- start: will trigger client initialization and registration to server\n");
        printf("- changed: will mark the resource /%d/%d/%d as updated, and trigger notifications if needed\n",
                CONFIG_RESOURCE_OBJ_ID, CONFIG_RESOURCE_INST_ID, CONFIG_RESOURCE_ID);
    return 1;
}
