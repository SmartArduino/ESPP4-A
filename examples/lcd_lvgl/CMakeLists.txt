cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../components/gc4023")
set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;components/gen_bmgr_codes/board_manager.defaults")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(lcd_lvgl)
