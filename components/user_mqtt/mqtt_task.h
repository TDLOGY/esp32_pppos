#ifndef MQTT_TASK_H_
#define MQTT_TASK_H_

#include <stdint.h>
#include <stdbool.h>

bool mqtt_connected(void);
bool mqtt_packet_create(uint32_t event, char* buf, uint32_t *length);
void mqtt_publish_status(uint8_t *data, uint16_t length);
void mqtt_init(void);

#endif