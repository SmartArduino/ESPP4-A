# Board information output
message(STATUS "Selected Board: esp32_p4_devkit")
message(STATUS "Board Path: <ESPP4-A>/components/espp4_a_boards/esp32_p4_devkit")

idf_component_register(
    SRC_DIRS "." "../../../../components/espp4_a_boards/esp32_p4_devkit"
    INCLUDE_DIRS "." "../../../../components/espp4_a_boards/esp32_p4_devkit"
    REQUIRES esp_board_manager
)

# This is equivalent to adding WHOLE_ARCHIVE option to the idf_component_register call above:
idf_component_set_property(${COMPONENT_NAME} WHOLE_ARCHIVE TRUE)
