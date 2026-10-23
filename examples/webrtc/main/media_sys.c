#include <string.h>
#include "esp_log.h"
#include "av_render.h"
#include "av_render_default.h"
#include "esp_audio_enc_default.h"
#include "esp_video_enc_default.h"
#include "esp_audio_dec_default.h"
#include "esp_capture_defaults.h"
#include "esp_board_manager_defs.h"
#include "esp_board_manager_includes.h"
#include "common.h"

static const char *TAG = "media_sys";

static esp_capture_handle_t s_capture;
static av_render_handle_t s_player;

static esp_codec_dev_handle_t get_codec(const char *name)
{
    dev_audio_codec_handles_t *codec = NULL;
    if (esp_board_device_get_handle(name, (void **)&codec) != ESP_OK) {
        return NULL;
    }
    return codec->codec_dev;
}

static int build_capture_system(void)
{

    dev_camera_handle_t *camera = NULL;
    if (esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_CAMERA, (void **)&camera) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get camera device");
        return -1;
    }
    esp_capture_video_v4l2_src_cfg_t v4l2_cfg = {
        .buf_count = 4,
    };
    strncpy(v4l2_cfg.dev_name, camera->dev_path, sizeof(v4l2_cfg.dev_name) - 1);
    esp_capture_video_src_if_t *vid_src = esp_capture_new_video_v4l2_src(&v4l2_cfg);
    if (vid_src == NULL) {
        return -1;
    }
    vid_src->set_fixed_caps(vid_src, &(esp_capture_video_info_t){
        .format_id = ESP_CAPTURE_FMT_ID_O_UYY_E_VYY,
        .width = VIDEO_WIDTH,
        .height = VIDEO_HEIGHT,
        .fps = VIDEO_FPS,
    });

    esp_codec_dev_handle_t record = get_codec(ESP_BOARD_DEVICE_NAME_AUDIO_ADC);
    esp_codec_dev_set_in_gain(record, 32);
    esp_capture_audio_dev_src_cfg_t codec_cfg = {
        .record_handle = record,
    };
    esp_capture_audio_src_if_t *aud_src = esp_capture_new_audio_dev_src(&codec_cfg);
    if (aud_src == NULL) {
        return -1;
    }

    esp_capture_cfg_t cfg = {
        .sync_mode = ESP_CAPTURE_SYNC_MODE_SYSTEM,
        .audio_src = aud_src,
        .video_src = vid_src,
    };
    return esp_capture_open(&cfg, &s_capture);
}

static int build_player_system(void)
{
    esp_codec_dev_handle_t play = get_codec(ESP_BOARD_DEVICE_NAME_AUDIO_DAC);
    esp_codec_dev_set_out_vol(play, 70);
    i2s_render_cfg_t i2s_cfg = {
        .fixed_clock = true,
        .play_handle = play,
    };
    av_render_cfg_t render_cfg = {
        .audio_render = av_render_alloc_i2s_render(&i2s_cfg),
        .audio_raw_fifo_size = 4096,
        .audio_render_fifo_size = 6 * 1024,
        .allow_drop_data = false,
    };
    s_player = av_render_open(&render_cfg);
    if (s_player == NULL) {
        ESP_LOGE(TAG, "Fail to create player");
        return -1;
    }

    av_render_audio_frame_info_t fixed_info = {
        .sample_rate = 8000,
        .channel = 2,
        .bits_per_sample = 16,
    };
    av_render_set_fixed_frame_info(s_player, &fixed_info);
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 8000,
        .channel = 2,
        .bits_per_sample = 16,
    };
    esp_codec_dev_open(play, &fs);
    return 0;
}

int media_sys_buildup(void)
{
    esp_video_enc_register_default();
    esp_audio_enc_register_default();
    esp_audio_dec_register_default();
    if (build_capture_system() != 0) {
        ESP_LOGE(TAG, "Fail to build capture system");
        return -1;
    }
    return build_player_system();
}

int media_sys_get_provider(esp_webrtc_media_provider_t *provide)
{
    provide->capture = s_capture;
    provide->player = s_player;
    return 0;
}
