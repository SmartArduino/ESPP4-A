/* This example code is in the Public Domain (CC0). */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_board_manager.h"
#include "esp_board_manager_defs.h"
#include "media_lib_adapter.h"
#include "media_lib_os.h"
#include "esp_capture.h"
#include "common.h"

static const char *TAG = "webrtc_main";

static char s_ip[16];

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting");
        s_ip[0] = '\0';
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip, sizeof(s_ip), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip:%s", s_ip);
    }
}

static esp_err_t wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed (%s)", esp_err_to_name(err));
        return err;
    }
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "connecting to WiFi %s ...", CONFIG_EXAMPLE_WIFI_SSID);
    return ESP_OK;
}

static void thread_scheduler(const char *thread_name, media_lib_thread_cfg_t *cfg)
{
    if (strcmp(thread_name, "venc_0") == 0) {
        cfg->priority = 10;
    } else if (strcmp(thread_name, "AUD_SRC") == 0) {
        cfg->priority = 15;
    } else if (strcmp(thread_name, "pc_task") == 0) {
        cfg->stack_size = 25 * 1024;
        cfg->priority = 18;
        cfg->core_id = 1;
    } else if (strcmp(thread_name, "start") == 0) {
        cfg->stack_size = 6 * 1024;
    } else if (strcmp(thread_name, "Adec") == 0) {
        cfg->stack_size = 40 * 1024;
        cfg->priority = 10;
        cfg->core_id = 1;
    }
}

static void capture_scheduler(const char *name, esp_capture_thread_schedule_cfg_t *schedule_cfg)
{
    media_lib_thread_cfg_t cfg = {
        .stack_size = schedule_cfg->stack_size,
        .priority = schedule_cfg->priority,
        .core_id = schedule_cfg->core_id,
    };
    schedule_cfg->stack_in_ext = true;
    thread_scheduler(name, &cfg);
    schedule_cfg->stack_size = cfg.stack_size;
    schedule_cfg->priority = cfg.priority;
    schedule_cfg->core_id = cfg.core_id;
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    media_lib_add_default_adapter();
    esp_capture_set_thread_scheduler(capture_scheduler);
    media_lib_thread_set_schedule_cb(thread_scheduler);

    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_AUDIO_ADC));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_AUDIO_DAC));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_CAMERA));
    ESP_ERROR_CHECK(esp_board_device_init("boot_button"));
    ESP_ERROR_CHECK(media_sys_buildup());

    if (wifi_init_sta() != ESP_OK) {
        return;
    }

    bool started = false;
    while (1) {
        if (s_ip[0] && !started) {
            started = start_webrtc() == 0;
            if (started) {
                ESP_LOGI(TAG, "open https://%s/webrtc/test in a browser to start a call", s_ip);
            }
        } else if (!s_ip[0] && started) {
            stop_webrtc();
            started = false;
        }
        query_webrtc();
        media_lib_thread_sleep(2000);
    }
}
