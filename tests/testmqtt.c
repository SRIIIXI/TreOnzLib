#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mqtt.h"

static volatile int g_integration_message_received = 0;

static void testmqtt_message_callback(const string_t* topic, const buffer_t* payload, void* userdata)
{
    (void)userdata;
    if (topic != NULL && payload != NULL)
    {
        g_integration_message_received = 1;
    }
}

static int env_to_int(const char* value, int fallback)
{
    if (value == NULL || value[0] == 0)
    {
        return fallback;
    }

    char* end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || *end != 0)
    {
        return fallback;
    }

    return (int)parsed;
}

int main(int argc, char* argv[])
{
    mqtt_client_t* client = NULL;
    buffer_t* payload = NULL;

    assert(mqtt_client_is_connected(NULL) == false);
    assert(mqtt_client_connect(NULL, "127.0.0.1", 1883, false, 60) == false);
    assert(mqtt_client_disconnect(NULL) == false);
    assert(mqtt_client_publish(NULL, "a/b", NULL, 0, false) == false);
    assert(mqtt_client_subscribe(NULL, "a/b", NULL, NULL) == false);
    assert(mqtt_client_unsubscribe(NULL, "a/b") == false);
    assert(mqtt_client_set_client_id(NULL, "dev-1") == false);
    assert(mqtt_client_set_username(NULL, "user") == false);
    assert(mqtt_client_set_password(NULL, "pass") == false);
    mqtt_client_clear_credentials(NULL);

    client = mqtt_client_allocate("treonz-test-client");
    assert(client != NULL);
    assert(mqtt_client_is_connected(client) == false);
    assert(mqtt_client_disconnect(client) == false);
    assert(mqtt_client_set_client_id(client, NULL) == false);
    assert(mqtt_client_set_client_id(client, "") == false);
    assert(mqtt_client_set_client_id(client, "device-42") == true);

    assert(mqtt_client_set_username(client, NULL) == false);
    assert(mqtt_client_set_username(client, "") == false);
    assert(mqtt_client_set_username(client, "iot-user") == true);

    assert(mqtt_client_set_password(client, NULL) == false);
    assert(mqtt_client_set_password(client, "") == false);
    assert(mqtt_client_set_password(client, "iot-pass") == true);
    mqtt_client_clear_credentials(client);

    payload = buffer_allocate("abc", 3);
    assert(payload != NULL);
    assert(mqtt_client_publish(client, "a/b", payload, 0, false) == false);
    assert(mqtt_client_subscribe(client, "a/b", NULL, NULL) == false);
    assert(mqtt_client_unsubscribe(client, "a/b") == false);

    mqtt_client_loop(client, 0);

    buffer_free(&payload);
    mqtt_client_free(&client);
    assert(client == NULL);

    /* Optional real broker integration block.
     * Enabled only when MQTT_HOST is provided in environment. */
    const char* mqtt_host = getenv("MQTT_HOST");
    const char* mqtt_port_str = getenv("MQTT_PORT");
    const char* mqtt_client_id = getenv("MQTT_CLIENT_ID");
    const char* mqtt_username = getenv("MQTT_USERNAME");
    const char* mqtt_password = getenv("MQTT_PASSWORD");
    const char* mqtt_topic = getenv("MQTT_TOPIC");
    const char* mqtt_use_tls_str = getenv("MQTT_USE_TLS");
    const char* mqtt_timeout_ms_str = getenv("MQTT_TIMEOUT_MS");

    if (mqtt_host != NULL && mqtt_host[0] != 0)
    {
        int mqtt_port = env_to_int(mqtt_port_str, 1883);
        int mqtt_use_tls = env_to_int(mqtt_use_tls_str, 0);
        int mqtt_timeout_ms = env_to_int(mqtt_timeout_ms_str, 3000);
        int elapsed_ms = 0;

        if (mqtt_client_id == NULL || mqtt_client_id[0] == 0)
        {
            mqtt_client_id = "treonz-it-client";
        }

        if (mqtt_topic == NULL || mqtt_topic[0] == 0)
        {
            mqtt_topic = "treonzlib/integration";
        }

        if (mqtt_password != NULL && mqtt_password[0] != 0 && (mqtt_username == NULL || mqtt_username[0] == 0))
        {
            fprintf(stderr, "MQTT integration config invalid: MQTT_PASSWORD provided without MQTT_USERNAME\n");
            return 2;
        }

        client = mqtt_client_allocate(mqtt_client_id);
        assert(client != NULL);

        if (mqtt_username != NULL && mqtt_username[0] != 0)
        {
            assert(mqtt_client_set_username(client, mqtt_username) == true);
        }
        if (mqtt_password != NULL && mqtt_password[0] != 0)
        {
            assert(mqtt_client_set_password(client, mqtt_password) == true);
        }

        assert(mqtt_client_connect(client, mqtt_host, mqtt_port, mqtt_use_tls != 0, 30) == true);
        assert(mqtt_client_is_connected(client) == true);

        g_integration_message_received = 0;
        assert(mqtt_client_subscribe(client, mqtt_topic, testmqtt_message_callback, NULL) == true);

        payload = buffer_allocate("treonz-integration", strlen("treonz-integration"));
        assert(payload != NULL);
        assert(mqtt_client_publish(client, mqtt_topic, payload, 0, false) == true);

        while (elapsed_ms < mqtt_timeout_ms && g_integration_message_received == 0)
        {
            mqtt_client_loop(client, 100);
            usleep(100 * 1000);
            elapsed_ms += 100;
        }

        buffer_free(&payload);
        assert(g_integration_message_received == 1);

        assert(mqtt_client_unsubscribe(client, mqtt_topic) == true);
        assert(mqtt_client_disconnect(client) == true);
        mqtt_client_free(&client);
        assert(client == NULL);
    }
    else
    {
        printf("MQTT integration skipped (set MQTT_HOST to enable)\n");
    }

    return 0;
}