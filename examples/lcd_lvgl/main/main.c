#include "esp_log.h"
#include "esp_board_manager_includes.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"

static const char *TAG = "lcd_lvgl";

#define LCD_H_RES 480
#define LCD_V_RES 854

void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_LCD_BRIGHTNESS));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD));
    ESP_ERROR_CHECK(esp_board_device_init(ESP_BOARD_DEVICE_NAME_LCD_TOUCH));

    dev_display_lcd_handles_t *lcd;
    dev_lcd_touch_handles_t *touch;
    periph_gpio_handle_t *backlight;
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, (void **)&lcd));
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_LCD_TOUCH, (void **)&touch));
    ESP_ERROR_CHECK(esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_LCD_BRIGHTNESS, (void **)&backlight));
    ESP_ERROR_CHECK(gpio_set_level(backlight->gpio_num, 1));
    ESP_LOGI(TAG, "Backlight on");

    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd->io_handle,
        .panel_handle = lcd->panel_handle,
        .buffer_size = LCD_H_RES * 100,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .color_format = LV_COLOR_FORMAT_RGB565,
    };
    lvgl_port_display_dsi_cfg_t dsi_cfg = {0};
    lv_display_t *disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);

    lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = touch->touch_handle,
    };
    lvgl_port_add_touch(&touch_cfg);

    lvgl_port_lock(0);
    lv_demo_widgets();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "LVGL widgets demo started");
}
