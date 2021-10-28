
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "my_mem.h"
#include "utils.h"

#define STORAGE_NAMESPACE       "storage"
#define STORAGE_WORKING_DATA    "working"
#define STORAGE_DEVICE_DATA     "device"

WorkingData_t workingData;
DeviceStore_t deviceStore;

const DeviceStore_t deviceStoreDefault = {
    .store_flag = STORAGE_FLAG_VALID,
    .domain = "test",
    .mqtt_url = "mqtt://test.mosquitto.org:1883",
    .CA_root =  "",
    .cert =     "",
    .priv_key =  "",
};


void my_mem_init(void)
{
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    storage_get_data(&workingData);
    device_get_data(&deviceStore);
}

bool storage_set_data(WorkingData_t *data)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    data->store_flag = STORAGE_FLAG_VALID;

    err = nvs_set_blob(my_handle, STORAGE_WORKING_DATA, data, sizeof(WorkingData_t));
    if (err != ESP_OK) return false;

    err = nvs_commit(my_handle);
    if (err != ESP_OK) return false;

    nvs_close(my_handle);
    return true;
}

bool storage_get_data(WorkingData_t *data)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    size_t required_size = sizeof(WorkingData_t);
    err = nvs_get_blob(my_handle, STORAGE_WORKING_DATA, data, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
        memset(data, 0, sizeof(WorkingData_t));
    }
    nvs_close(my_handle);

    if(data->store_flag != STORAGE_FLAG_VALID)
    {
        data->store_flag = STORAGE_FLAG_VALID;
        storage_set_data(data);
    }
    return true;
}

bool device_set_data(DeviceStore_t *data)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    data->store_flag = STORAGE_FLAG_VALID;

    err = nvs_set_blob(my_handle, STORAGE_DEVICE_DATA, data, sizeof(DeviceStore_t));
    if (err != ESP_OK) return false;

    err = nvs_commit(my_handle);
    if (err != ESP_OK) return false;

    nvs_close(my_handle);
    return true;
}

bool device_get_data(DeviceStore_t *data)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    size_t required_size = sizeof(DeviceStore_t);
    err = nvs_get_blob(my_handle, STORAGE_DEVICE_DATA, data, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
        memset(data, 0, sizeof(DeviceStore_t));
    }
    nvs_close(my_handle);

    /*  Set default data    */
    if((data->store_flag != STORAGE_FLAG_VALID) 
        || (data->device_id[0] == 0))
    {
        memcpy(data, &deviceStoreDefault, sizeof(DeviceStore_t));
        strcpy(data->device_id, flatform_get_id());
        device_set_data(data);
    }

    return true;
}
