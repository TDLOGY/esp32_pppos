set(srcs "mqtt_task.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES driver mqtt https_ota utils my_mem)
