cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    "$ENV{IDF_PATH}/examples/bluetooth/blufi/main")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ESP32-P4 has no on-chip Bluetooth; define this so the IDF BluFi example compiles.
idf_build_set_property(COMPILE_DEFINITIONS "CONFIG_SOC_BLUFI_SUPPORTED=1" APPEND)

project(blufi)
