# espp4_a_boards：ESPP4-A 板级定义（esp_board_manager）

[esp_board_manager](https://components.espressif.com/components/espressif/esp_board_manager) 是乐鑫的板级管理组件：
用 YAML 描述板子的外设和设备，生成初始化代码，应用里按名字获取设备句柄，例如：

```c
esp_board_device_init(ESP_BOARD_DEVICE_NAME_AUDIO_DAC);
dev_audio_codec_handles_t *codec;
esp_board_device_get_handle(ESP_BOARD_DEVICE_NAME_AUDIO_DAC, (void **)&codec);
```

适配之后，乐鑫基于 board manager 的例程（esp-webrtc-solution 等）只需少量修改就能在本板上运行。

## esp32_p4_devkit（ESP32-P4 DevKit v1.1）

板目录：`esp32_p4_devkit/`

| 文件 | 内容 |
| --- | --- |
| `board_info.yaml` | 板名、芯片 |
| `board_peripherals.yaml` | 外设：I2C ×3、I2S、GPIO、LDO、DSI |
| `board_devices.yaml` | 设备：音频、按键、屏、触摸、背光、摄像头、TF 卡 |
| `setup_device.c` | ST7102 屏和触摸的创建函数，以及 TF 卡的时钟设置与上电等待 |
| `sdkconfig.defaults.board` | 板级 sdkconfig（Flash、PSRAM、编解码芯片、摄像头时钟） |

### 设备

| 设备名 | 类型 | 说明 |
| --- | --- | --- |
| `audio_dac` / `audio_adc` | audio_codec | ES8311 播放 / 录音，I2S0 TDM 全双工，功放常开 |
| `boot_button` | button | BOOT 键 GPIO35 |
| `display_lcd` | display_lcd (dsi) | ST7102 480×854，DSI 2-lane 440 Mbps |
| `lcd_touch` | lcd_touch (i2c) | 与屏同一颗芯片，按 ST7123 协议 |
| `lcd_brightness` | gpio_ctrl | 背光使能 GPIO24 |
| `camera` | camera (csi) | GC4023，驱动在 [components/gc4023](../gc4023/) |
| `fs_sdcard` | fs_fat (sdmmc) | TF 卡，Slot 0、4 线，挂载到 `/sdcard`，失败不格式化 |
| `sdcard_power_ctrl` | power_ctrl (gpio) | GPIO4 低电平供电 |

### I2C 分配

DevKit 上有三组独立的 I2C，而 P4 只有两个 HP I2C，所以摄像头放在 LP I2C 上
（GPIO9/10 属于 LP GPIO 0~15，可以路由过去）：

| 外设名 | 端口 | 引脚 | 设备 |
| --- | --- | --- | --- |
| `i2c_audio` | I2C0 | SDA=28 SCL=25 | ES8311 |
| `i2c_touch` | I2C1 | SDA=49 SCL=48 | 触摸 |
| `i2c_camera` | LP_I2C0 | SDA=9 SCL=10 | GC4023 |

`setup_device.c` 在 `esp_board_runtime_pre_init()` 里设置 TF 卡时钟（YAML 频率字段只接受 `SDMMC_FREQ_*`），
并注册挂载前 50 ms 的上电等待。
卡检测为 GPIO45，不开启片内 LDO4。
`fs_sdcard` 设置了 `init_skip: true`，全板初始化不要求插卡，使用时显式调用
`esp_board_device_init(ESP_BOARD_DEVICE_NAME_FS_SDCARD)`。

### 无线接口

- **WiFi / BLE**：由 [p4_wireless](../p4_wireless/) 提供，不归 board manager 管。

## 在工程里使用

1. 工程 `main/idf_component.yml` 添加依赖（依赖名以 `_boards` 结尾，board manager 会自动扫描这个组件里的板子）：
   ```yaml
   dependencies:
     espressif/esp_board_manager: "~0.7.4"
     espp4_a_boards:
       path: <相对 main/ 的路径>/components/espp4_a_boards
   ```
2. 生成板级代码（写到工程的 `components/gen_bmgr_codes/`）：
   ```sh
   idf.py reconfigure
   IDF_EXTRA_ACTIONS_PATH=$PWD/managed_components/espressif__esp_board_manager \
       idf.py bmgr -b esp32_p4_devkit
   ```
   装了 `esp-bmgr-assist`（`pip install esp-bmgr-assist`）时可以直接 `idf.py bmgr -b esp32_p4_devkit`。
3. 完整板定义包含摄像头设备，把 [components/gc4023](../gc4023/) 加进 `EXTRA_COMPONENT_DIRS`，
   并将 `components/gen_bmgr_codes/board_manager.defaults` 加进 `SDKCONFIG_DEFAULTS`。
   示例已经包含这些设置。应用按需调用 `esp_board_device_init()`，只初始化实际用到的设备。

示例见 [audio](../../examples/audio/)、[lcd_lvgl](../../examples/lcd_lvgl/)、
[camera](../../examples/camera/)、[sdcard](../../examples/sdcard/) 和 [webrtc](../../examples/webrtc/)。
每个工程的 `components/gen_bmgr_codes/` 都由同一份板定义生成。修改 YAML 后需要在使用该板定义的工程中重新生成；
`setup_device.c` 由生成组件直接编译，修改后重新构建即可。
