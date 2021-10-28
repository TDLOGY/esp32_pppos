set(srcs "pppos_task.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES driver modem)
