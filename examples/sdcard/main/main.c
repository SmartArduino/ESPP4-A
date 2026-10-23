#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_board_manager_includes.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"

static const char *TAG = "sdcard";

#define MOUNT_POINT "/sdcard"
#define FILE_PATH   MOUNT_POINT "/hello.txt"

void app_main(void)
{
    esp_err_t ret = esp_board_device_init(ESP_BOARD_DEVICE_NAME_FS_SDCARD);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mount failed: %s", esp_err_to_name(ret));
        return;
    }

    uint64_t total = 0, free_bytes = 0;
    esp_vfs_fat_info(MOUNT_POINT, &total, &free_bytes);
    ESP_LOGI(TAG, "mounted, total %llu MB, free %llu MB", total / 1024 / 1024, free_bytes / 1024 / 1024);

    FILE *f = fopen(FILE_PATH, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "open %s for write failed", FILE_PATH);
        goto out;
    }
    fprintf(f, "Hello from ESPP4-A!\n");
    fclose(f);
    ESP_LOGI(TAG, "wrote %s", FILE_PATH);

    char line[64] = {0};
    f = fopen(FILE_PATH, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "open %s for read failed", FILE_PATH);
        goto out;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    line[strcspn(line, "\n")] = '\0';
    ESP_LOGI(TAG, "read back: \"%s\"", line);

    ESP_LOGI(TAG, "files in %s:", MOUNT_POINT);
    DIR *dir = opendir(MOUNT_POINT);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            char path[300];
            struct stat st;
            snprintf(path, sizeof(path), MOUNT_POINT "/%s", entry->d_name);
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                ESP_LOGI(TAG, "  %-24s <DIR>", entry->d_name);
            } else {
                ESP_LOGI(TAG, "  %-24s %ld bytes", entry->d_name, (long)st.st_size);
            }
        }
        closedir(dir);
    }

out:
    ESP_ERROR_CHECK(esp_board_device_deinit(ESP_BOARD_DEVICE_NAME_FS_SDCARD));
    ESP_LOGI(TAG, "unmounted");
}
