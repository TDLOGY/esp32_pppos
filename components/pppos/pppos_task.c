#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"
#include "esp_log.h"
#include "sim7600.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "pppos_task.h"

#define TIMEOUT_PPP_DISCONNECTED    (60000U)            // 60s
#define TIMEOUT_PPP_CONNECTED       (20000U)            // 60s

typedef enum{
    MODULE_SIMCOM = 0,
    MODULE_QUECTEL = 1,
}ModuleType_t;

static const char *TAG = "pppos";

static EventGroupHandle_t event_group = NULL;
static const int CONNECT_BIT = BIT0;
static const int STOP_BIT = BIT1;
static const int DISCONNECT_BIT = BIT2;
static volatile bool pppos_connected = false;
static ModuleType_t moduleType = MODULE_SIMCOM;
static modem_dce_t *dce = NULL;

static void modem_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
    case ESP_MODEM_EVENT_PPP_START:
        ESP_LOGI(TAG, "Modem PPP Started");
        break;
    case ESP_MODEM_EVENT_PPP_STOP:
        ESP_LOGI(TAG, "Modem PPP Stopped");
        xEventGroupSetBits(event_group, STOP_BIT);
        pppos_connected = false;
        break;
    case ESP_MODEM_EVENT_UNKNOWN:
        ESP_LOGW(TAG, "Unknow line received: %s", (char *)event_data);
        break;
    default:
        break;
    }
}

static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
    // ESP_LOGI(TAG, "PPP state changed event %d", event_id);
    if (event_id == NETIF_PPP_ERRORUSER) {
        /* User interrupted event from esp-netif */
        esp_netif_t *netif = *(esp_netif_t**)event_data;
        ESP_LOGI(TAG, "User interrupted event from netif:%p", netif);

        xEventGroupSetBits(event_group, DISCONNECT_BIT);
    }
    else if(event_id == NETIF_PPP_ERRORCONNECT)
    {
        ESP_LOGI(TAG, "NETIF_PPP_ERRORCONNECT");
        xEventGroupSetBits(event_group, DISCONNECT_BIT);
    }
}

static void on_ip_event(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    // ESP_LOGD(TAG, "IP event! %d", event_id);
    if (event_id == IP_EVENT_PPP_GOT_IP) {
        // esp_netif_dns_info_t dns_info;

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        // esp_netif_t *netif = event->esp_netif;

        // ESP_LOGI(TAG, "Modem Connect to PPP Server");
        ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
        ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
        // ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
        // ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.gw));
        // esp_netif_get_dns_info(netif, 0, &dns_info);
        // ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        // esp_netif_get_dns_info(netif, 1, &dns_info);
        // ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
        xEventGroupSetBits(event_group, CONNECT_BIT);
        pppos_connected = true;
    } else if (event_id == IP_EVENT_PPP_LOST_IP) {
        ESP_LOGI(TAG, "Modem Disconnect from PPP Server");
        pppos_connected = false;
    } else if (event_id == IP_EVENT_GOT_IP6) {
        ESP_LOGI(TAG, "GOT IPv6 event!");

        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
    }
}

void SIM_reset(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << 15;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_set_level(15, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    gpio_set_level(15, 1);
}

void pppos_task_init(void)
{
    SIM_reset();

    vTaskDelay(pdMS_TO_TICKS(10000));

    event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));

    /* create dte object */
    esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    config.tx_io_num = 17;
    config.rx_io_num = 16;
    config.event_task_stack_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_STACK_SIZE;
    modem_dte_t *dte = esp_modem_dte_init(&config);

    /* Register event handler */
    ESP_ERROR_CHECK(esp_modem_set_event_handler(dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

    // Init netif object
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_PPP();
    esp_netif_t *esp_netif = esp_netif_new(&cfg);
    assert(esp_netif);

    void *modem_netif_adapter = esp_modem_netif_setup(dte);
    esp_modem_netif_set_default_handlers(modem_netif_adapter, esp_netif);

    uint8_t cnt = 0;
    do
    {
        dce = sim7600_init(dte);
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        if(++cnt >= 20)
        {
            cnt = 0;
            SIM_reset();
        }
    }
    while (dce == NULL);
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", dce->name);
    ESP_LOGI(TAG, "Operator: %s", dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", dce->imsi);

    if(strstr(dce->name, "EC600S")){
        moduleType = MODULE_QUECTEL;
    }
    uint32_t rssi = 0, ber = 0;

    /* Get signal quality */
    for(int i = 0; i < 20; i++){
        dce->get_signal_quality(dce, &rssi, &ber);
        if((rssi != 99) && (rssi != 0))
        {
            break;
        }
        ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    dce->store_profile(dce);

    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);

    /* attach the modem to the network interface */
    esp_netif_attach(esp_netif, modem_netif_adapter);

    /* Wait for IP address */
    xEventGroupWaitBits(event_group, CONNECT_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(TIMEOUT_PPP_CONNECTED)); 
}

void pppos_reset(void)
{
    ESP_LOGI(TAG, "PPP Reset");
    esp_modem_dte_reinit(dce->dte);
    SIM_reset();
    dce->mode = MODEM_COMMAND_MODE;
    xEventGroupWaitBits(event_group, DISCONNECT_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(TIMEOUT_PPP_DISCONNECTED));

    for(int i = 0; i < 20; i++){
        ESP_LOGI(TAG, "Sync...");
        if(sim7600_reinit(dce)){
            ESP_LOGI(TAG, "Sync OK");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    dce->store_profile(dce);

    uint32_t rssi = 0, ber = 0;
    dce->get_signal_quality(dce, &rssi, &ber);
    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
    
    ESP_LOGI(TAG, "Start PPP");
    esp_modem_start_ppp(dce->dte);
    xEventGroupWaitBits(event_group, CONNECT_BIT, pdTRUE, pdTRUE,  pdMS_TO_TICKS(TIMEOUT_PPP_CONNECTED)); 
}

bool pppos_is_connected(void)
{
    return pppos_connected;
}

char* pppos_get_imei(void)
{
    return dce->imei;
}

char* pppos_get_imsi(void)
{
    return dce->imsi;
}