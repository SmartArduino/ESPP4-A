/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * Auto-generated peripheral configuration file
 * DO NOT MODIFY THIS FILE MANUALLY
 *
 * See LICENSE file for details.
 */

#include <stdlib.h>
#include "esp_board_periph.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "hal/gpio_types.h"
#include "periph_gpio.h"
#include "periph_i2s.h"

// Peripheral configuration structures
const static i2c_master_bus_config_t esp_bmgr_i2c_audio_cfg = {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = 28,
    .scl_io_num = 25,
    .glitch_ignore_cnt = 7,
    .intr_priority = 1,
    .trans_queue_depth = 0,
    .flags = {
        .enable_internal_pullup = true,
    },
    .clk_source = I2C_CLK_SRC_DEFAULT,
};

const static i2c_master_bus_config_t esp_bmgr_i2c_camera_cfg = {
    .i2c_port = LP_I2C_NUM_0,
    .sda_io_num = 9,
    .scl_io_num = 10,
    .glitch_ignore_cnt = 7,
    .intr_priority = 1,
    .trans_queue_depth = 0,
    .flags = {
        .enable_internal_pullup = true,
    },
    .lp_source_clk = LP_I2C_SCLK_DEFAULT,
};

const static i2c_master_bus_config_t esp_bmgr_i2c_touch_cfg = {
    .i2c_port = I2C_NUM_1,
    .sda_io_num = 49,
    .scl_io_num = 48,
    .glitch_ignore_cnt = 7,
    .intr_priority = 1,
    .trans_queue_depth = 0,
    .flags = {
        .enable_internal_pullup = true,
    },
    .clk_source = I2C_CLK_SRC_DEFAULT,
};

const static periph_i2s_config_t esp_bmgr_i2s_audio_out_cfg = {
    .port = I2S_NUM_0,
    .role = I2S_ROLE_MASTER,
    .mode = I2S_COMM_MODE_TDM,
    .direction = I2S_DIR_TX,
    .i2s_cfg = {
        .tdm = {
            .clk_cfg = {
                .sample_rate_hz = 48000,
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .ext_clk_freq_hz = 0,
                .mclk_multiple = I2S_MCLK_MULTIPLE_256,
                .bclk_div = 8,
            },
            .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
                .slot_mode = I2S_SLOT_MODE_STEREO,
                .slot_mask = I2S_TDM_SLOT0,
                .ws_width = 16,
                .ws_pol = false,
                .bit_shift = true,
                .left_align = false,
                .big_endian = false,
                .bit_order_lsb = false,
                .skip_mask = false,
                .total_slot = 2,
            },
            .gpio_cfg = {
                .mclk = 26,
                .bclk = 27,
                .ws = 30,
                .dout = 31,
                .din = 29,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        },
    },
};

const static periph_i2s_config_t esp_bmgr_i2s_audio_in_cfg = {
    .port = I2S_NUM_0,
    .role = I2S_ROLE_MASTER,
    .mode = I2S_COMM_MODE_TDM,
    .direction = I2S_DIR_RX,
    .i2s_cfg = {
        .tdm = {
            .clk_cfg = {
                .sample_rate_hz = 48000,
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .ext_clk_freq_hz = 0,
                .mclk_multiple = I2S_MCLK_MULTIPLE_256,
                .bclk_div = 8,
            },
            .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
                .slot_mode = I2S_SLOT_MODE_STEREO,
                .slot_mask = I2S_TDM_SLOT0 | I2S_TDM_SLOT1,
                .ws_width = 16,
                .ws_pol = false,
                .bit_shift = true,
                .left_align = false,
                .big_endian = false,
                .bit_order_lsb = false,
                .skip_mask = false,
                .total_slot = 2,
            },
            .gpio_cfg = {
                .mclk = 26,
                .bclk = 27,
                .ws = 30,
                .dout = 31,
                .din = 29,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        },
    },
};

const static periph_gpio_config_t esp_bmgr_gpio_boot_button_cfg = {
    .gpio_config = {
        .pin_bit_mask = BIT64(35),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    .default_level = 0,
};

const static periph_gpio_config_t esp_bmgr_gpio_backlight_cfg = {
    .gpio_config = {
        .pin_bit_mask = BIT64(24),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    .default_level = 0,
};

const static periph_gpio_config_t esp_bmgr_gpio_sd_power_cfg = {
    .gpio_config = {
        .pin_bit_mask = BIT64(4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    .default_level = 1,
};

const static esp_ldo_channel_config_t esp_bmgr_ldo_mipi_cfg = {
    .chan_id = 3,
    .voltage_mv = 2500,
    .flags = {
        .adjustable = 1,
        .owned_by_hw = 0,
    },
};

const static esp_lcd_dsi_bus_config_t esp_bmgr_dsi_display_cfg = {
    .bus_id = 0,
    .num_data_lanes = 2,
    .phy_clk_src = 0,
    .lane_bit_rate_mbps = 440,
};

// Peripheral descriptor array
const esp_board_periph_desc_t g_esp_board_peripherals[] = {
    {
        .next = &g_esp_board_peripherals[1],
        .name = "i2c_audio",
        .type = "i2c",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_MASTER,
        .cfg = &esp_bmgr_i2c_audio_cfg,
        .cfg_size = sizeof(esp_bmgr_i2c_audio_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[2],
        .name = "i2c_camera",
        .type = "i2c",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_MASTER,
        .cfg = &esp_bmgr_i2c_camera_cfg,
        .cfg_size = sizeof(esp_bmgr_i2c_camera_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[3],
        .name = "i2c_touch",
        .type = "i2c",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_MASTER,
        .cfg = &esp_bmgr_i2c_touch_cfg,
        .cfg_size = sizeof(esp_bmgr_i2c_touch_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[4],
        .name = "i2s_audio_out",
        .type = "i2s",
        .format = "tdm-out",
        .role = ESP_BOARD_PERIPH_ROLE_MASTER,
        .cfg = &esp_bmgr_i2s_audio_out_cfg,
        .cfg_size = sizeof(esp_bmgr_i2s_audio_out_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[5],
        .name = "i2s_audio_in",
        .type = "i2s",
        .format = "tdm-in",
        .role = ESP_BOARD_PERIPH_ROLE_MASTER,
        .cfg = &esp_bmgr_i2s_audio_in_cfg,
        .cfg_size = sizeof(esp_bmgr_i2s_audio_in_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[6],
        .name = "gpio_boot_button",
        .type = "gpio",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_IO,
        .cfg = &esp_bmgr_gpio_boot_button_cfg,
        .cfg_size = sizeof(esp_bmgr_gpio_boot_button_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[7],
        .name = "gpio_backlight",
        .type = "gpio",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_IO,
        .cfg = &esp_bmgr_gpio_backlight_cfg,
        .cfg_size = sizeof(esp_bmgr_gpio_backlight_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[8],
        .name = "gpio_sd_power",
        .type = "gpio",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_IO,
        .cfg = &esp_bmgr_gpio_sd_power_cfg,
        .cfg_size = sizeof(esp_bmgr_gpio_sd_power_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = &g_esp_board_peripherals[9],
        .name = "ldo_mipi",
        .type = "ldo",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_NONE,
        .cfg = &esp_bmgr_ldo_mipi_cfg,
        .cfg_size = sizeof(esp_bmgr_ldo_mipi_cfg),
        .id = 0,
        .init_skip = false,
    },
    {
        .next = NULL,
        .name = "dsi_display",
        .type = "dsi",
        .format = NULL,
        .role = ESP_BOARD_PERIPH_ROLE_NONE,
        .cfg = &esp_bmgr_dsi_display_cfg,
        .cfg_size = sizeof(esp_bmgr_dsi_display_cfg),
        .id = 0,
        .init_skip = false,
    },
};
