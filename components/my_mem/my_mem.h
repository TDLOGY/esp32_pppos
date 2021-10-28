#ifndef MY_MEM_H_
#define MY_MEM_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t store_flag;
    char device_id[64];
    char CA_root[2560];
    char cert[2560];
    char priv_key[2560];
    char domain[32];
    char mqtt_url[128];
}DeviceStore_t;

typedef struct
{
    uint32_t store_flag;
    uint32_t testData;
}WorkingData_t;

#define STORAGE_FLAG_VALID          (0x5A5A5A5A)

extern const DeviceStore_t deviceStoreDefault;
extern WorkingData_t workingData;
extern DeviceStore_t deviceStore;

void my_mem_init(void);

bool storage_get_data(WorkingData_t *data);
bool storage_set_data(WorkingData_t *data);

bool device_get_data(DeviceStore_t *data);
bool device_set_data(DeviceStore_t *data);

#endif