set(srcs "utils.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES app_update pppos esp_timer)
