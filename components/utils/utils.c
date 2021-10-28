#include <string.h>
#include <stdlib.h>
#include "esp_ota_ops.h"
#include "esp_system.h"

#include "esp_timer.h"

static char deviceID[32];

char* app_get_version(void)
{
    return esp_ota_get_app_description()->version;
}

void device_get_id(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(deviceID, "esp32_%02X%02X%02X", mac[3], mac[4], mac[5]);
}

char* flatform_get_id(void)
{
    if(deviceID[0] == '\0')
    {
        device_get_id();
    }
    return deviceID;
}

uint32_t flatform_get_boot_time(void)
{
    return esp_timer_get_time() / 1000000;
}
