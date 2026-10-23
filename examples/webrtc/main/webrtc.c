#include <string.h>
#include "esp_board_manager_includes.h"
#include "esp_log.h"
#include "media_lib_os.h"
#include "esp_webrtc_defaults.h"
#include "esp_peer_default.h"
#include "webrtc_http_server.h"
#include "common.h"

#define TAG "webrtc"

#define SAME_STR(a, b) (strncmp(a, b, sizeof(b) - 1) == 0)
#define SEND_CMD(cmd)  esp_webrtc_send_custom_data(s_webrtc, ESP_WEBRTC_CUSTOM_DATA_VIA_SIGNALING, (uint8_t *)cmd, strlen(cmd))

int esp_peer_set_peer_prefer_codec(esp_peer_handle_t handle, esp_peer_codec_caps_t codec_caps);

static esp_webrtc_handle_t s_webrtc;
static volatile bool s_key_running;

static int on_cmd(esp_webrtc_custom_data_via_t via, uint8_t *data, int size, void *ctx)
{
    if (via != ESP_WEBRTC_CUSTOM_DATA_VIA_SIGNALING || size == 0 || s_webrtc == NULL) {
        return 0;
    }
    ESP_LOGI(TAG, "Receive command %.*s", size, (char *)data);
    const char *cmd = (const char *)data;
    if (SAME_STR(cmd, "ACCEPT_CALL")) {
        esp_webrtc_enable_peer_connection(s_webrtc, true);

        esp_peer_handle_t peer = NULL;
        esp_webrtc_get_peer_connection(s_webrtc, &peer);
        esp_peer_set_peer_prefer_codec(peer, ESP_PEER_CAPS_AUD_OPUS);
    } else if (SAME_STR(cmd, "DENY_CALL")) {
        esp_webrtc_enable_peer_connection(s_webrtc, false);
    } else if (SAME_STR(cmd, "OPEN_DOOR")) {
        SEND_CMD("DOOR_OPENED");
    }
    return 0;
}

static int on_data(esp_peer_data_frame_t *frame, void *ctx)
{
    ESP_LOGI(TAG, "Data channel %d recv %d bytes", frame->stream_id, frame->size);
    return 0;
}

static int on_event(esp_webrtc_event_t *event, void *ctx)
{
    if (event->type == ESP_WEBRTC_EVENT_PAIRED) {
        esp_peer_handle_t peer = NULL;
        esp_webrtc_get_peer_connection(s_webrtc, &peer);
        esp_peer_addr_t addr = {};
        esp_peer_get_paired_addr(peer, &addr);
        ESP_LOGI(TAG, "Paired with %d.%d.%d.%d:%d", addr.ipv4[0], addr.ipv4[1], addr.ipv4[2], addr.ipv4[3], addr.port);
    } else if (event->type == ESP_WEBRTC_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "call established");
    } else if (event->type == ESP_WEBRTC_EVENT_DISCONNECTED || event->type == ESP_WEBRTC_EVENT_CONNECT_FAILED) {
        ESP_LOGI(TAG, "call ended (event %d)", event->type);
    }
    return 0;
}

static void key_thread(void *arg)
{
    dev_button_handles_t *button;
    ESP_ERROR_CHECK(esp_board_device_get_handle("boot_button", (void **)&button));
    int last = 0;
    while (s_key_running) {
        media_lib_thread_sleep(50);
        int level = iot_button_get_key_level(button->button_handles[0]);
        if (level != last && level == 1 && s_webrtc) {
            ESP_LOGI(TAG, "Ring");
            SEND_CMD("RING");
        }
        last = level;
    }
    media_lib_thread_destroy(NULL);
}

int start_webrtc(void)
{
    esp_peer_default_cfg_t peer_cfg = {
        .agent_recv_timeout = 500,
        .twcc_cfg = {
            .enable = true,
        },
    };
    esp_webrtc_cfg_t cfg = {
        .peer_cfg = {
            .audio_info = {
                .codec = ESP_PEER_AUDIO_CODEC_G711A,
                .sample_rate = 8000,
                .channel = 1,
            },
            .video_info = {
                .codec = ESP_PEER_VIDEO_CODEC_H264,
                .width = VIDEO_WIDTH,
                .height = VIDEO_HEIGHT,
                .fps = VIDEO_FPS,
            },
            .codec_caps = ESP_PEER_CAPS_AUD_OPUS | ESP_PEER_CAPS_AUD_G711A,
            .audio_dir = ESP_PEER_MEDIA_DIR_SEND_RECV,
            .video_dir = ESP_PEER_MEDIA_DIR_SEND_ONLY,
            .on_custom_data = on_cmd,
            .on_data = on_data,
            .enable_data_channel = true,
            .manual_ch_create = true,
            .no_auto_reconnect = true,
            .extra_cfg = &peer_cfg,
            .extra_size = sizeof(peer_cfg),
        },
        .peer_impl = esp_peer_get_default_impl(),
        .signaling_impl = esp_signaling_get_http_impl(),
    };
    int ret = esp_webrtc_open(&cfg, &s_webrtc);
    if (ret != 0) {
        ESP_LOGE(TAG, "Fail to open webrtc");
        return ret;
    }
    esp_webrtc_media_provider_t media_provider = {};
    media_sys_get_provider(&media_provider);
    esp_webrtc_set_media_provider(s_webrtc, &media_provider);
    esp_webrtc_set_event_handler(s_webrtc, on_event, NULL);
    esp_webrtc_enable_peer_connection(s_webrtc, false);

    ret = esp_webrtc_start(s_webrtc);
    if (ret != 0) {
        ESP_LOGE(TAG, "Fail to start webrtc");
        esp_webrtc_close(s_webrtc);
        s_webrtc = NULL;
        return ret;
    }
    s_key_running = true;
    media_lib_thread_handle_t key;
    media_lib_thread_create_from_scheduler(&key, "Key", key_thread, NULL);
    return 0;
}

void query_webrtc(void)
{
    if (s_webrtc) {
        esp_webrtc_query(s_webrtc);
    }
}

int stop_webrtc(void)
{
    if (s_webrtc) {
        s_key_running = false;
        esp_webrtc_handle_t handle = s_webrtc;
        s_webrtc = NULL;
        esp_webrtc_close(handle);
    }
    return 0;
}
