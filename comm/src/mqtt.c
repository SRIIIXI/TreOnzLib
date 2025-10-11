/*
BSD 2-Clause License

Copyright (c) 2025, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "mqtt.h"
#include <stdlib.h>
#include <string.h>
#include <time.h> /* for retry timestamp */

typedef struct mqtt_pending_pub_t
{
    uint16_t packet_id;
    buffer_t* payload;
    char* topic;
    time_t last_sent;
    struct mqtt_pending_pub_t* next;
} mqtt_pending_pub_t;

/* ======================== Subscription Structure ======================== */

typedef struct mqtt_subscription_t
{
    string_t* topic;
    mqtt_message_callback_t callback;
    void* userdata;
    struct mqtt_subscription_t* next;
} mqtt_subscription_t;

/* ======================== MQTT Client Structure ======================== */

struct mqtt_client_t
{
    string_t* client_id;
    tcp_client_t* tcp;
    int keepalive;
    bool connected;
    mqtt_subscription_t* subscription_head;
    uint16_t next_packet_id;       /* for QoS 1 */
    mqtt_pending_pub_t* pending;   /* linked list of pending PUBs */
};

/* ======================== MQTT Packet Helpers ======================== */

/* Encode Remaining Length per MQTT spec */
static size_t mqtt_encode_remaining_length(uint8_t* buffer, size_t length)
{
    size_t idx = 0;
    do
    {
        uint8_t byte = length % 128;
        length /= 128;
        if (length > 0)
        {
            byte |= 0x80;
        }
        buffer[idx++] = byte;
    } while (length > 0);
    return idx;
}

/* Build CONNECT packet */
static buffer_t* mqtt_build_connect_packet(const string_t* client_id, int keepalive)
{
    if (client_id == NULL)
    {
        return NULL;
    }

    buffer_t* buf = buffer_allocate_default();
    if (!buf)
    {
        return NULL;
    }

    const char* protocol_name = "MQTT";
    uint8_t protocol_level = 4; /* MQTT 3.1.1 */
    uint8_t connect_flags = 0x02; /* Clean session */
    uint16_t ka = (uint16_t)keepalive;

    buffer_append(buf, "\x00\x04", 2); /* Protocol Name length */
    buffer_append(buf, protocol_name, 4);
    buffer_append(buf, &protocol_level, 1);
    buffer_append(buf, &connect_flags, 1);
    uint8_t ka_bytes[2] = { (uint8_t)(ka >> 8), (uint8_t)(ka & 0xFF) };
    buffer_append(buf, ka_bytes, 2);

    /* Client ID */
    uint16_t client_id_len = (uint16_t)string_get_length(client_id);
    uint8_t cid_bytes[2] = { (uint8_t)(client_id_len >> 8), (uint8_t)(client_id_len & 0xFF) };
    buffer_append(buf, cid_bytes, 2);
    buffer_append(buf, string_c_str(client_id), client_id_len);

    /* Fixed Header */
    size_t remaining_len = buffer_get_size(buf);
    uint8_t fixed_header[5];
    fixed_header[0] = 0x10; /* CONNECT */
    size_t fh_len = mqtt_encode_remaining_length(fixed_header + 1, remaining_len);

    buffer_t* packet = buffer_allocate_length(fh_len + 1);
    buffer_append(packet, fixed_header, fh_len + 1);
    buffer_append(packet, buffer_get_data(buf), remaining_len);
    buffer_free(&buf);

    return packet;
}

/* Build PUBLISH packet (QoS 0) */
static buffer_t* mqtt_build_publish_packet_qos(const char* topic, const buffer_t* payload, uint8_t qos, uint16_t packet_id)
{
    if (!topic || !payload) return NULL;

    buffer_t* buf = buffer_allocate_default();

    /* Topic */
    uint16_t topic_len = (uint16_t)strlen(topic);
    uint8_t tlen_bytes[2] = { (uint8_t)(topic_len >> 8), (uint8_t)(topic_len & 0xFF) };
    buffer_append(buf, tlen_bytes, 2);
    buffer_append(buf, topic, topic_len);

    /* Packet ID for QoS 1 */
    if (qos == 1)
    {
        uint8_t pid_bytes[2] = { (uint8_t)(packet_id >> 8), (uint8_t)(packet_id & 0xFF) };
        buffer_append(buf, pid_bytes, 2);
    }

    /* Payload */
    buffer_append(buf, buffer_get_data(payload), buffer_get_size(payload));

    /* Fixed header */
    uint8_t fixed_header[5];
    fixed_header[0] = 0x30 | (qos << 1); /* QoS 0=0x30, QoS1=0x32 */
    size_t remaining_len = buffer_get_size(buf);
    size_t fh_len = mqtt_encode_remaining_length(fixed_header + 1, remaining_len);

    buffer_t* packet = buffer_allocate_length(fh_len + 1);
    buffer_append(packet, fixed_header, fh_len + 1);
    buffer_append(packet, buffer_get_data(buf), remaining_len);
    buffer_free(&buf);

    return packet;
}

/* Build DISCONNECT packet */
static buffer_t* mqtt_build_disconnect_packet()
{
    uint8_t packet[2] = { 0xE0, 0x00 };
    buffer_t* buf = buffer_allocate_length(2);
    buffer_append(buf, packet, 2);
    return buf;
}

/* Build SUBSCRIBE packet */
static buffer_t* mqtt_build_subscribe_packet(uint16_t packet_id, const char* topic)
{
    buffer_t* buf = buffer_allocate_default();

    uint8_t pid_bytes[2] = { (uint8_t)(packet_id >> 8), (uint8_t)(packet_id & 0xFF) };
    buffer_append(buf, pid_bytes, 2);

    uint16_t topic_len = (uint16_t)strlen(topic);
    uint8_t tlen_bytes[2] = { (uint8_t)(topic_len >> 8), (uint8_t)(topic_len & 0xFF) };
    buffer_append(buf, tlen_bytes, 2);
    buffer_append(buf, topic, topic_len);

    uint8_t qos_byte = 0x00;
    buffer_append(buf, &qos_byte, 1);

    size_t remaining_len = buffer_get_size(buf);
    uint8_t fixed_header[5];
    fixed_header[0] = 0x82; /* SUBSCRIBE flags */
    size_t fh_len = mqtt_encode_remaining_length(fixed_header + 1, remaining_len);

    buffer_t* packet = buffer_allocate_length(fh_len + 1);
    buffer_append(packet, fixed_header, fh_len + 1);
    buffer_append(packet, buffer_get_data(buf), remaining_len);
    buffer_free(&buf);

    return packet;
}

/* ======================== Subscription Helpers ======================== */

static mqtt_subscription_t* mqtt_add_subscription(mqtt_client_t* client, const char* topic, mqtt_message_callback_t cb, void* userdata)
{
    mqtt_subscription_t* sub = (mqtt_subscription_t*)malloc(sizeof(mqtt_subscription_t));
    sub->topic = string_allocate(topic);
    sub->callback = cb;
    sub->userdata = userdata;
    sub->next = client->subscription_head;
    client->subscription_head = sub;
    return sub;
}

static void mqtt_remove_subscription(mqtt_client_t* client, const char* topic)
{
    mqtt_subscription_t* prev = NULL;
    mqtt_subscription_t* curr = client->subscription_head;

    while (curr)
    {
        if (strcmp(string_c_str(curr->topic), topic) == 0)
        {
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                client->subscription_head = curr->next;
            }
            string_free(&curr->topic);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ======================== Client Lifecycle ======================== */

mqtt_client_t* mqtt_client_allocate(const char* client_id)
{
    mqtt_client_t* client = (mqtt_client_t*)malloc(sizeof(mqtt_client_t));
    client->client_id = string_allocate(client_id);
    client->tcp = tcp_client_allocate();
    client->keepalive = 60;
    client->connected = false;
    client->subscription_head = NULL;
    return client;
}

void mqtt_client_free(mqtt_client_t** client)
{
    if (!client || !*client)
    {
        return;
    }

    mqtt_client_t* ptr = *client;
    if (ptr->tcp) tcp_client_free(ptr->tcp);
    if (ptr->client_id) string_free(&ptr->client_id);

    mqtt_subscription_t* sub = ptr->subscription_head;
    while (sub)
    {
        mqtt_subscription_t* next = sub->next;
        string_free(&sub->topic);
        free(sub);
        sub = next;
    }

    free(ptr);
    *client = NULL;
}

/* ======================== Connection ======================== */

bool mqtt_client_connect(mqtt_client_t* client, const char* host, int port, bool use_tls, int keepalive_seconds)
{
    if (!client) return false;
    client->keepalive = keepalive_seconds;

    if (!tcp_client_create_socket(client->tcp, host, port)) return false;
    if (!tcp_client_connect_socket(client->tcp)) return false;
    if (use_tls && !tcp_client_switch_to_tls(client->tcp)) return false;

    buffer_t* packet = mqtt_build_connect_packet(client->client_id, client->keepalive);
    if (!packet) return false;

    bool sent = tcp_client_send_buffer(client->tcp, packet);
    buffer_free(&packet);
    if (!sent) return false;

    buffer_t* resp = tcp_client_receive_buffer_by_length(client->tcp, 4);
    if (!resp || buffer_get_size(resp) < 4)
    {
        buffer_free(&resp);
        return false;
    }

    const uint8_t* data = (const uint8_t*)buffer_get_data(resp);
    bool success = (data[0] == 0x20 && data[1] == 0x02 && data[3] == 0x00);
    buffer_free(&resp);

    client->connected = success;
    return success;
}

bool mqtt_client_disconnect(mqtt_client_t* client)
{
    if (!client || !client->connected) return false;

    buffer_t* packet = mqtt_build_disconnect_packet();
    if (packet)
    {
        tcp_client_send_buffer(client->tcp, packet);
        buffer_free(&packet);
    }

    tcp_client_close_socket(client->tcp);
    client->connected = false;
    return true;
}

bool mqtt_client_is_connected(mqtt_client_t* client)
{
    return client && client->connected && tcp_client_is_connected(client->tcp);
}

/* ======================== Publish/Subscribe ======================== */

bool mqtt_client_publish(mqtt_client_t* client, const char* topic, const buffer_t* payload, uint8_t qos, bool retain)
{
    if (!client || !client->connected || !topic || !payload) return false;

    uint16_t pid = 0;
    if (qos == 1)
    {
        pid = client->next_packet_id++;
        if (client->next_packet_id == 0) client->next_packet_id = 1; /* wrap */
    }

    buffer_t* packet = mqtt_build_publish_packet_qos(topic, payload, qos, pid);
    if (!packet) return false;

    bool sent = tcp_client_send_buffer(client->tcp, packet);
    buffer_free(&packet);
    if (!sent) return false;

    /* If QoS 1, store in pending list until PUBACK */
    if (qos == 1)
    {
        mqtt_pending_pub_t* pub = (mqtt_pending_pub_t*)malloc(sizeof(mqtt_pending_pub_t));
        pub->packet_id = pid;

        /* Allocate a new buffer of the same size */
        size_t payload_size = buffer_get_size((buffer_t*)payload);
        pub->payload = buffer_allocate_length(payload_size);
        buffer_append(pub->payload, buffer_get_data((buffer_t*)payload), payload_size);

        pub->topic = strdup(topic);
        pub->next = client->pending;
        client->pending = pub;
    }

    return true;
}

bool mqtt_client_subscribe(mqtt_client_t* client, const char* topic, mqtt_message_callback_t callback, void* userdata)
{
    if (!client || !client->connected || !topic || !callback) return false;

    mqtt_add_subscription(client, topic, callback, userdata);

    static uint16_t packet_id = 1;
    buffer_t* packet = mqtt_build_subscribe_packet(packet_id++, topic);
    if (!packet) return false;

    bool sent = tcp_client_send_buffer(client->tcp, packet);
    buffer_free(&packet);
    return sent;
}

bool mqtt_client_unsubscribe(mqtt_client_t* client, const char* topic)
{
    if (!client || !client->connected || !topic)
    {
        return false;
    }

    /* Remove from internal subscription list */
    mqtt_remove_subscription(client, topic);

    /* Build UNSUBSCRIBE packet */
    static uint16_t packet_id = 1;
    buffer_t* buf = buffer_allocate_default();
    if (!buf) return false;

    /* Packet ID */
    uint8_t pid_bytes[2] = { (uint8_t)(packet_id >> 8), (uint8_t)(packet_id & 0xFF) };
    buffer_append(buf, pid_bytes, 2);

    /* Topic */
    uint16_t topic_len = (uint16_t)strlen(topic);
    uint8_t tlen_bytes[2] = { (uint8_t)(topic_len >> 8), (uint8_t)(topic_len & 0xFF) };
    buffer_append(buf, tlen_bytes, 2);
    buffer_append(buf, topic, topic_len);

    /* Fixed Header */
    size_t remaining_len = buffer_get_size(buf);
    uint8_t fixed_header[5];
    fixed_header[0] = 0xA2; /* UNSUBSCRIBE, flags 0x2 */
    size_t fh_len = mqtt_encode_remaining_length(fixed_header + 1, remaining_len);

    buffer_t* packet = buffer_allocate_length(fh_len + 1);
    buffer_append(packet, fixed_header, fh_len + 1);
    buffer_append(packet, buffer_get_data(buf), remaining_len);
    buffer_free(&buf);

    /* Send UNSUBSCRIBE */
    bool sent = tcp_client_send_buffer(client->tcp, packet);
    buffer_free(&packet);

    packet_id++; /* Increment for next packet */
    return sent;
}


/* ======================== Event Loop ======================== */

void mqtt_client_loop(mqtt_client_t* client, int timeout_ms)
{
    if (!client || !client->connected) return;

    buffer_t* incoming = tcp_client_receive_buffer_by_length(client->tcp, 2);
    if (incoming && buffer_get_size(incoming) >= 2)
    {
        const uint8_t* data = (const uint8_t*)buffer_get_data(incoming);
        uint8_t packet_type = data[0] >> 4;

        if (packet_type == 3) /* PUBLISH */
        {
            size_t idx = 1, multiplier = 1, remaining_len = 0;
            uint8_t encodedByte;
            do
            {
                encodedByte = data[idx++];
                remaining_len += (encodedByte & 127) * multiplier;
                multiplier *= 128;
            } while ((encodedByte & 128) != 0 && idx < buffer_get_size(incoming));

            buffer_free(&incoming);
            buffer_t* payload_buf = tcp_client_receive_buffer_by_length(client->tcp, remaining_len);
            if (!payload_buf) return;

            const uint8_t* p_data = (const uint8_t*)buffer_get_data(payload_buf);
            uint16_t topic_len = (p_data[0] << 8) | p_data[1];
            char topic_name[256];
            memcpy(topic_name, p_data + 2, topic_len);
            topic_name[topic_len] = 0;

            size_t payload_offset = 2 + topic_len;
            uint16_t packet_id = 0;
            if ((data[0] & 0x06) >> 1 == 1) /* QoS 1 */
            {
                packet_id = (p_data[payload_offset] << 8) | p_data[payload_offset + 1];
                payload_offset += 2;

                /* Send PUBACK */
                uint8_t puback[4] = {0x40, 0x02, p_data[payload_offset - 2], p_data[payload_offset - 1]};
                buffer_t* ack_buf = buffer_allocate_length(4);
                buffer_append(ack_buf, puback, 4);
                tcp_client_send_buffer(client->tcp, ack_buf);
                buffer_free(&ack_buf);
            }

            size_t message_len = remaining_len - payload_offset;
            const uint8_t* message = p_data + payload_offset;

            mqtt_subscription_t* sub = client->subscription_head;
            while (sub)
            {
                if (strcmp(string_c_str(sub->topic), topic_name) == 0)
                {
                    buffer_t* msg_buf = buffer_allocate_length(message_len);
                    buffer_append(msg_buf, message, message_len);
                    sub->callback(sub->topic, msg_buf, sub->userdata);
                    buffer_free(&msg_buf);
                }
                sub = sub->next;
            }

            buffer_free(&payload_buf);
        }
        else if (packet_type == 4) /* PUBACK */
        {
            uint16_t ack_id = (data[2] << 8) | data[3];
            mqtt_pending_pub_t* prev = NULL;
            mqtt_pending_pub_t* curr = client->pending;
            while (curr)
            {
                if (curr->packet_id == ack_id)
                {
                    if (prev)
                        prev->next = curr->next;
                    else
                        client->pending = curr->next;

                    buffer_free(&curr->payload);
                    free(curr->topic);
                    free(curr);
                    break;
                }
                prev = curr;
                curr = curr->next;
            }
            buffer_free(&incoming);
        }
        else
        {
            buffer_free(&incoming);
        }
    }

    /* =================== Retry unacknowledged QoS1 messages =================== */
    time_t now = time(NULL);
    mqtt_pending_pub_t* curr = client->pending;
    while (curr)
    {
        if (difftime(now, curr->last_sent) >= 5) /* 5 seconds retry interval */
        {
            buffer_t* packet = mqtt_build_publish_packet_qos(curr->topic, curr->payload, 1, curr->packet_id);
            if (packet)
            {
                tcp_client_send_buffer(client->tcp, packet);
                buffer_free(&packet);
                curr->last_sent = now;
            }
        }
        curr = curr->next;
    }
}
