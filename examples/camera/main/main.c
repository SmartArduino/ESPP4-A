#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/ppa.h"
#include "esp_board_manager_includes.h"
#include "esp_cache.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "linux/videodev2.h"

static const char *TAG = "camera";

#define LCD_H_RES 480
#define LCD_V_RES 854
#define CAM_W     1920
#define CAM_H     1080
#define CAM_BUFS  3

static SemaphoreHandle_t s_vsync_sem;

static bool on_refresh_done(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_vsync_sem, &woken);
    return woken == pdTRUE;
}

static void video_ioctl(int fd, int request, void *arg)
{
    if (ioctl(fd, request, arg) != 0) {
        ESP_LOGE(TAG, "ioctl 0x%x failed: %s", request, strerror(errno));
        abort();
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_LCD_BRIGHTNESS));

    dev_display_lcd_config_t *lcd_config;
    ESP_ERROR_CHECK(esp_board_device_get_config(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, (void **)&lcd_config));
    dev_display_lcd_config_t preview_config = *lcd_config;
    // Three frame buffers so PPA never writes the one still being scanned out
    preview_config.sub_cfg.dsi.dpi_config.num_fbs = 3;
    ESP_ERROR_CHECK(esp_board_device_override_config(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, &preview_config, sizeof(preview_config)));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD));

    dev_display_lcd_handles_t *lcd;
    periph_gpio_handle_t *backlight;
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, (void **)&lcd));
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_LCD_BRIGHTNESS, (void **)&backlight));
    esp_lcd_panel_handle_t panel = lcd->panel_handle;
    void *fbs[3];
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_get_frame_buffer(panel, 3, &fbs[0], &fbs[1], &fbs[2]));
    size_t fb_size = LCD_H_RES * LCD_V_RES * 2;
    for (int i = 0; i < 3; i++) {

        memset(fbs[i], 0, fb_size);
        ESP_ERROR_CHECK(esp_cache_msync(fbs[i], fb_size, ESP_CACHE_MSYNC_FLAG_DIR_C2M));
    }
    s_vsync_sem = xSemaphoreCreateBinary();
    assert(s_vsync_sem);
    esp_lcd_dpi_panel_event_callbacks_t dpi_cbs = {
        .on_refresh_done = on_refresh_done,
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(panel, &dpi_cbs, NULL));
    ESP_ERROR_CHECK(gpio_set_level(backlight->gpio_num, 1));
    ESP_LOGI(TAG, "LCD ready");

    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_CAMERA));
    dev_camera_handle_t *camera;
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_CAMERA, (void **)&camera));
    int fd = open(camera->dev_path, O_RDWR);
    if (fd < 0) {
        ESP_LOGE(TAG, "open %s failed: %s", camera->dev_path, strerror(errno));
        return;
    }
    struct v4l2_format format = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .fmt.pix = {
            .width = CAM_W,
            .height = CAM_H,
            .pixelformat = V4L2_PIX_FMT_RGB565,
        },
    };
    video_ioctl(fd, VIDIOC_S_FMT, &format);
    assert(format.fmt.pix.width == CAM_W && format.fmt.pix.height == CAM_H &&
           format.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565);
    struct v4l2_requestbuffers req = {
        .count = CAM_BUFS,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP,
    };
    video_ioctl(fd, VIDIOC_REQBUFS, &req);
    assert(req.count == CAM_BUFS);
    void *buffers[CAM_BUFS];
    for (int i = 0; i < CAM_BUFS; i++) {
        struct v4l2_buffer buf = {
            .index = i,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP,
        };
        video_ioctl(fd, VIDIOC_QUERYBUF, &buf);
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        assert(buffers[i] && buffers[i] != MAP_FAILED);
        video_ioctl(fd, VIDIOC_QBUF, &buf);
    }

    ppa_client_handle_t ppa;
    ppa_client_config_t ppa_config = {
        .oper_type = PPA_OPERATION_SRM,
    };
    ESP_ERROR_CHECK(ppa_register_client(&ppa_config, &ppa));

    ppa_srm_oper_config_t srm = {
        .in = {
            .pic_w = CAM_W,
            .pic_h = CAM_H,
            .block_w = CAM_W,
            .block_h = CAM_H,
            .srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        },
        .out = {
            .buffer_size = fb_size,
            .pic_w = LCD_H_RES,
            .pic_h = LCD_V_RES,
            .block_offset_x = (LCD_H_RES - CAM_H * 7 / 16) / 2,
            .block_offset_y = (LCD_V_RES - CAM_W * 7 / 16) / 2,
            .srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        },
        .rotation_angle = PPA_SRM_ROTATION_ANGLE_90,
        .scale_x = 7.0f / 16,
        .scale_y = 7.0f / 16,
        .mode = PPA_TRANS_MODE_BLOCKING,
    };
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_ioctl(fd, VIDIOC_STREAMON, &type);
    ESP_LOGI(TAG, "Streaming on, %dx%d", CAM_W, CAM_H);

    int back = 1;
    uint32_t frames = 0;
    int64_t last_log = esp_timer_get_time();
    while (1) {
        struct v4l2_buffer buf = {
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP,
        };
        video_ioctl(fd, VIDIOC_DQBUF, &buf);
        if (!(buf.flags & V4L2_BUF_FLAG_ERROR)) {
            srm.in.buffer = buffers[buf.index];
            srm.out.buffer = fbs[back];
            ESP_ERROR_CHECK(ppa_do_scale_rotate_mirror(ppa, &srm));
            // Reuse the old frame buffer only after the LCD has switched buffers.
            xSemaphoreTake(s_vsync_sem, 0);
            ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, fbs[back]));
            if (xSemaphoreTake(s_vsync_sem, pdMS_TO_TICKS(100)) != pdTRUE) {
                ESP_LOGE(TAG, "LCD refresh timeout");
                abort();
            }
            back = (back + 1) % 3;
            frames++;
        }
        // Return the capture buffer only after PPA has finished reading it.
        video_ioctl(fd, VIDIOC_QBUF, &buf);

        int64_t now = esp_timer_get_time();
        if (now - last_log >= 1000000) {
            ESP_LOGI(TAG, "display fps: %.1f", frames * 1000000.0 / (now - last_log));
            frames = 0;
            last_log = now;
        }
    }
}
