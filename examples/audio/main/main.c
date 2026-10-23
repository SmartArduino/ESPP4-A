#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_board_manager_includes.h"
#include "esp_codec_dev.h"
#include "esp_log.h"

static const char *TAG = "audio";

#define SAMPLE_RATE   16000
#define CHUNK_SAMPLES (SAMPLE_RATE / 50 * 2)

void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_AUDIO_DAC));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_AUDIO_ADC));
    dev_audio_codec_handles_t *dac, *adc;
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_AUDIO_DAC, (void **)&dac));
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_AUDIO_ADC, (void **)&adc));

    esp_codec_dev_sample_info_t sample_cfg = {
        .bits_per_sample = 16,
        .channel = 2,
        .channel_mask = 0x03,
        .sample_rate = SAMPLE_RATE,
    };
    ESP_ERROR_CHECK(esp_codec_dev_open(dac->codec_dev, &sample_cfg));
    ESP_ERROR_CHECK(esp_codec_dev_open(adc->codec_dev, &sample_cfg));
    ESP_ERROR_CHECK(esp_codec_dev_set_out_vol(dac->codec_dev, 60));
    ESP_ERROR_CHECK(esp_codec_dev_set_in_gain(adc->codec_dev, 30.0));
    ESP_LOGI(TAG, "ES8311 ready");

    static int16_t tone[SAMPLE_RATE / 10 * 2];  // static: keep the large buffer off the main task stack
    for (int i = 0; i < SAMPLE_RATE / 10; i++) {
        int16_t v = (int16_t)(8000 * sinf(2 * M_PI * 1000 * i / SAMPLE_RATE));
        tone[i * 2] = v;
        tone[i * 2 + 1] = v;
    }
    ESP_LOGI(TAG, "play 1kHz tone for 2s");
    for (int i = 0; i < 20; i++) {
        ESP_ERROR_CHECK(esp_codec_dev_write(dac->codec_dev, tone, sizeof(tone)));
    }

    ESP_LOGI(TAG, "mic -> speaker loopback, speak now");
    static int16_t buf[CHUNK_SAMPLES];
    while (1) {
        ESP_ERROR_CHECK(esp_codec_dev_read(adc->codec_dev, buf, sizeof(buf)));
        // Channel 2 is the DAC loopback reference, play back the mic channel only.
        for (int i = 0; i < CHUNK_SAMPLES; i += 2) {
            buf[i + 1] = buf[i];
        }
        ESP_ERROR_CHECK(esp_codec_dev_write(dac->codec_dev, buf, sizeof(buf)));
    }
}
