idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES p4_wireless console esp_wifi esp_netif esp_event nvs_flash lwip)
