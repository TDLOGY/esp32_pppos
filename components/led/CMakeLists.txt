set(srcs "led.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES driver)
