# Pass the GC4023 IPA (auto exposure / white balance) config to esp_ipa
idf_build_set_property(ESP_IPA_JSON_CONFIG_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/cfg/gc4023_default.json" APPEND)
