set(srcs "https_ota.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES esp_http_client esp_https_ota json app_update  utils my_mem)
