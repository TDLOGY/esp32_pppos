#include <string.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "mqtt_task.h"
#include "led.h"
#include "pppos_task.h"
#include "utils.h"
#include "my_mem.h"

typedef enum
{
    MODE_START = 0,
    MODE_CONNECTING,
    MODE_CONNECTED,
    MODE_RECONNECT_INIT,
}MainState_t;

static const char* TAG = "main";

static MainState_t mainState;

void log_init(void)
{
    esp_log_level_set("*", ESP_LOG_ERROR);        // set all components to ERROR level
    esp_log_level_set("modem", ESP_LOG_INFO);        // set all components to ERROR level
    esp_log_level_set("mqtt task", ESP_LOG_INFO);      // enable WARN logs from WiFi stack
    esp_log_level_set("pppos", ESP_LOG_INFO);     // enable INFO logs from DHCP client
    esp_log_level_set("https_ota", ESP_LOG_NONE);     // enable INFO logs from DHCP client
    esp_log_level_set("main", ESP_LOG_INFO);     // enable INFO logs from DHCP client
}

void main_state_machine(void)
{
    uint32_t cntCheckConnect = 0;

    switch (mainState)
    {
    case MODE_START:
        /*  SIM7600 init with PPPOS */
        pppos_task_init();
        mqtt_init();
        mainState = MODE_CONNECTING;
        break;
    case MODE_CONNECTING:
        ESP_LOGI(TAG, "Connecting...");
        cntCheckConnect = 0;
        while(!mqtt_connected()){
            vTaskDelay(pdMS_TO_TICKS(1000));
            if(++cntCheckConnect >= 20){
                mainState = MODE_RECONNECT_INIT;
                break;
            }
        }
        /*  -------------- MQTT Connected ------------------ */
        if(mqtt_connected())
        {
            ESP_LOGI(TAG, "Connected");
            mainState = MODE_CONNECTED;
        }
        else
        {
            mainState = MODE_RECONNECT_INIT;
        }
        break;
    case MODE_CONNECTED:
        while (true)
        {
            vTaskDelay(100);
            if(!mqtt_connected())
            {
                mainState = MODE_RECONNECT_INIT;
            }
        }    
        break;
    case MODE_RECONNECT_INIT:
        // Reconnect
        ESP_LOGI(TAG, "Reconnecting...");
        pppos_reset();
        mainState = MODE_CONNECTING;
        break;
    default:
        break;
    }
}

void app_main(void)
{
    log_init();
    esp_netif_init();
    esp_event_loop_create_default();
    my_mem_init();

    /*  LED init */
    leds_init();

    while (true)
    {
        main_state_machine();
    }
}
