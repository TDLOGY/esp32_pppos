#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "mqtt_task.h"
#include "https_ota.h"
#include "utils.h"
#include "my_mem.h"

extern EventGroupHandle_t event_group;

static esp_mqtt_client_handle_t mqtt_client;
static volatile bool mqttConnected = false;
static const char *TAG = "mqtt task";

char* mqtt_get_topic_status(void)
{
    static char topic[128];
    sprintf(topic, "%s/%s/status",  deviceStore.domain, deviceStore.device_id);
    return topic;
}

char* mqtt_get_topic_control(void)
{
    static char topic[128];
    sprintf(topic, "%s/%s/control", deviceStore.domain, deviceStore.device_id);
    return topic;
}

char* mqtt_get_topic_fw(void)
{
    static char topic[128];
    sprintf(topic, "%s/%s/fw",  deviceStore.domain, deviceStore.device_id);
    return topic;
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id; (void)msg_id;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, mqtt_get_topic_control(), 1);
        msg_id = esp_mqtt_client_subscribe(client, mqtt_get_topic_fw(), 1);
        mqttConnected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqttConnected = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        if(memcmp(mqtt_get_topic_fw(), event->topic, event->topic_len) == 0)
        {
            app_ota_packet(event->data, event->data_len);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        // ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    default:
        ESP_LOGI(TAG, "MQTT other event id: %d", event->event_id);
        break;
    }
    return ESP_OK;
}

bool mqtt_connected(void)
{
    return mqttConnected;
}

void mqtt_publish_status(uint8_t *data, uint16_t length)
{
    if(!mqttConnected) return;
    int msg_id = esp_mqtt_client_publish(mqtt_client, mqtt_get_topic_status(), (char*)data, 0, 1, 0);
    // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

    (void)msg_id;
}

void mqtt_init(void)
{
    char *device_id = deviceStore.device_id;

    char lwt_msg[256];
    uint16_t lwt_msg_len = 0;

	lwt_msg_len += sprintf(lwt_msg + lwt_msg_len, "{\"id\":\"%s\",", device_id);
	lwt_msg_len += sprintf(lwt_msg + lwt_msg_len, "\"return\":201,");
	lwt_msg_len += sprintf(lwt_msg + lwt_msg_len, "\"fw\": \"%s\"}", app_get_version());

    ESP_LOGI(TAG, "\n----------------------------------------\n");
    ESP_LOGI(TAG, "Device ID    : %s", device_id);
    ESP_LOGI(TAG, "TopicStatus  : %s", mqtt_get_topic_status());
    ESP_LOGI(TAG, "TopicControl : %s", mqtt_get_topic_control());
    ESP_LOGI(TAG, "TopicFirmware: %s", mqtt_get_topic_fw());
    ESP_LOGI(TAG, "MQTT URL     : %s", deviceStore.mqtt_url);
    ESP_LOGI(TAG, "\n----------------------------------------");

    /* Config MQTT */
    esp_mqtt_client_config_t mqtt_config = {
        .uri = deviceStore.mqtt_url,
        .client_cert_pem = deviceStore.cert,
        .client_key_pem = deviceStore.priv_key,
        .cert_pem = deviceStore.CA_root,
        .keepalive = 120,
        .event_handle = mqtt_event_handler,
        .client_id = device_id,
        .lwt_topic = mqtt_get_topic_status(),
        .lwt_qos = 1,
        .lwt_retain = false,
        .lwt_msg = lwt_msg,
        .lwt_msg_len = lwt_msg_len,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_start(mqtt_client);   
}