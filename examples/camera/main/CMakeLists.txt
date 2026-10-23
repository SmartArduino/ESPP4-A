idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_board_manager gen_bmgr_codes gc4023 esp_lcd esp_driver_ppa esp_mm esp_timer)
