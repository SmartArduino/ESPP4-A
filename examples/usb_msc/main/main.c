#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "usb/usb_host.h"
#include "usb/msc_host.h"
#include "usb/msc_host_vfs.h"

static const char *TAG = "usb_msc";

#define MOUNT_PATH "/usb"
#define TEST_FILE  MOUNT_PATH "/p4test.bin"
#define CHUNK      (64 * 1024)
#define TEST_BYTES (32 * 1024 * 1024)

static QueueHandle_t s_event_queue;

static void msc_event_cb(const msc_host_event_t *event, void *arg)
{
    xQueueSend(s_event_queue, event, 0);
}

static void usb_lib_task(void *arg)
{
    const usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .peripheral_map = BIT0,  // High-Speed OTG (USB_DP/USB_DM)
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    xTaskNotifyGive((TaskHandle_t)arg);
    while (1) {
        uint32_t flags;
        usb_host_lib_handle_events(portMAX_DELAY, &flags);
    }
}

static void log_speed(const char *what, int64_t start_us)
{
    float seconds = (esp_timer_get_time() - start_us) / 1000000.0f;
    ESP_LOGI(TAG, "%s %d MB in %.2f s, %.2f MB/s", what, TEST_BYTES >> 20, seconds, (TEST_BYTES >> 20) / seconds);
}

static void file_test(uint8_t *buf)
{
    DIR *dir = opendir(MOUNT_PATH);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            ESP_LOGI(TAG, "  %s", entry->d_name);
        }
        closedir(dir);
    }

    int fd = open(TEST_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        ESP_LOGE(TAG, "Failed to create %s", TEST_FILE);
        return;
    }
    int64_t start = esp_timer_get_time();
    for (int done = 0; done < TEST_BYTES; done += CHUNK) {
        if (write(fd, buf, CHUNK) != CHUNK) {
            ESP_LOGE(TAG, "File write failed");
            break;
        }
    }
    fsync(fd);
    close(fd);
    log_speed("Write", start);

    fd = open(TEST_FILE, O_RDONLY);
    start = esp_timer_get_time();
    for (int done = 0; done < TEST_BYTES; done += CHUNK) {
        if (read(fd, buf, CHUNK) != CHUNK) {
            ESP_LOGE(TAG, "File read failed");
            break;
        }
    }
    close(fd);
    log_speed("Read", start);
    unlink(TEST_FILE);
}

void app_main(void)
{
    s_event_queue = xQueueCreate(4, sizeof(msc_host_event_t));
    xTaskCreate(usb_lib_task, "usb_lib", 4096, xTaskGetCurrentTaskHandle(), 10, NULL);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    const msc_host_driver_config_t msc_config = {
        .create_backround_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = tskNO_AFFINITY,
        .callback = msc_event_cb,
    };
    ESP_ERROR_CHECK(msc_host_install(&msc_config));
    uint8_t *buf = malloc(CHUNK);
    assert(buf);
    ESP_LOGI(TAG, "Waiting for a USB storage device");

    msc_host_vfs_handle_t vfs = NULL;
    while (1) {
        msc_host_event_t event;
        xQueueReceive(s_event_queue, &event, portMAX_DELAY);
        if (event.event == MSC_DEVICE_DISCONNECTED) {
            if (vfs) {
                msc_host_vfs_unregister(vfs);
                vfs = NULL;
            }
            msc_host_uninstall_device(event.device.handle);
            ESP_LOGI(TAG, "Device disconnected");
            continue;
        }
        if (event.event != MSC_DEVICE_CONNECTED) {
            continue;
        }

        msc_host_device_handle_t dev;
        if (msc_host_install_device(event.device.address, &dev) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to install the MSC device");
            continue;
        }
        msc_host_device_info_t info;
        ESP_ERROR_CHECK(msc_host_get_device_info(dev, &info));
        ESP_LOGI(TAG, "VID 0x%04X PID 0x%04X, %ls %ls", info.idVendor, info.idProduct, info.iManufacturer, info.iProduct);
        ESP_LOGI(TAG, "Capacity %.1f GB", (double)info.sector_count * info.sector_size / 1e9);

        const esp_vfs_fat_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 3,
        };
        if (msc_host_vfs_register(dev, MOUNT_PATH, &mount_config, &vfs) != ESP_OK) {
            vfs = NULL;
            ESP_LOGW(TAG, "Mount failed: only an MBR partition table with a FAT32 partition is supported");
            continue;
        }
        ESP_LOGI(TAG, "Mounted at %s", MOUNT_PATH);
        file_test(buf);
    }
}
