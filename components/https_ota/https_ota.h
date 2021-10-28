#ifndef HTTPS_OTA_H_
#define HTTPS_OTA_H_

#include <stdint.h>

bool app_ota_start(char* newVer, char* url);
void app_ota_packet(char *data, uint32_t len);

#endif