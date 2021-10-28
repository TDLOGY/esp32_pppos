set(srcs "my_mem.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash utils)
