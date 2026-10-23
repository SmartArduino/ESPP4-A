idf_component_register(SRCS "gc4023.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_cam_sensor esp_driver_gpio)
# The driver is only found by esp_cam_sensor through a linker section, so force it to be linked
target_link_libraries(${COMPONENT_LIB} INTERFACE "-u gc4023_detect")
