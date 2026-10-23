# gc4023：GC4023 摄像头驱动（esp_cam_sensor）

格科微 GC4023 的 [esp_cam_sensor](https://components.espressif.com/components/espressif/esp_cam_sensor) 驱动，
`esp_cam_sensor` 官方目前不支持这颗传感器。加入工程后会被 `esp_video` 自动探测，可以和乐鑫的
`esp_video` / `esp_capture` / `esp_board_manager` 配合使用。

| 项目 | 参数 |
| --- | --- |
| 接口 | MIPI-CSI 2-lane，720 Mbps/lane |
| 输出 | RAW10 1920×1080 25fps（从 2560×1440 居中裁剪） |
| MCLK | 24 MHz |
| SCCB 地址 | 0x29 |
| 芯片 ID | 0x4023（寄存器 0x03F0/0x03F1） |

## 使用

工程 `CMakeLists.txt` 的 `EXTRA_COMPONENT_DIRS` 加上本目录即可，不需要调用任何函数：

- 驱动通过 `ESP_CAM_SENSOR_DETECT_FN` 注册，`esp_video` 初始化时自动探测
- `project_include.cmake` 会把 `cfg/gc4023_default.json` 交给 `esp_ipa`，由 ISP 自动做曝光、白平衡等

## 来源与说明

- 寄存器表来自 HiSpark hi_aiot_solution_adapter 的 galaxycore_gc4023，在 DevKit 上改为 1920×1080 输出；
  帧长改为 1650 行，使帧率正好 25fps。
- 修改输出窗口（0x0094~0x009B）时，必须同时修改 MIPI 行字节数 0x010d/0x010e = 宽 × 10 / 8，
  否则 ISP 报 `MIPI_HNUM_UNMATCH`。
- `cfg/gc4023_default.json` 目前借用 GC4053 的调参数据，颜色和曝光可能需要按实际画面调整。
