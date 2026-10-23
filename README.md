中文 | [English](README_EN.md)

# ESPP4-A

ESPP4-A 是基于乐鑫 **ESP32-P4** 的模组。ESP32-P4 本身没有无线功能，模组内置无线芯片，提供 **WiFi（2.4 / 5 GHz）+ BLE**。
应用使用 ESP-IDF 标准的 `esp_wifi`、`esp_netif` 和 NimBLE 接口，写法与 ESP32 片上 WiFi 相同。

本仓库包含模组和开发板资料、开发环境搭建教程，以及各外设的例程。每个例程的 README 就是该外设的使用说明。

## 快速开始

```sh
source ~/esp/esp-idf/export.sh      # 换成你的 ESP-IDF 路径，要求 v5.5 及以上
cd components/p4_wireless/examples/wifi_cmd
idf.py set-target esp32p4
idf.py -p /dev/ttyUSB0 flash monitor
```

启动后在 `wifi>` 提示符下输入 `sta "WiFi名称" "密码"`，串口输出 `got ip:` 即表示模组和 WiFi 工作正常。第一次使用 ESP-IDF 请先看 [开发环境搭建](docs/03_开发环境搭建.md)。

## 文档

| 文档 | 内容 |
| --- | --- |
| [01_模组介绍](docs/01_模组介绍.md) | 规格、方案优势（集成度、吞吐对比）、引脚定义、模组内部占用的引脚 |
| [02_开发板介绍](docs/02_开发板介绍.md) | DevKit v1.1 板载外设与 GPIO 分配 |
| [03_开发环境搭建](docs/03_开发环境搭建.md) | 安装 ESP-IDF、编译、烧录、看日志 |
| [04_常见问题](docs/04_常见问题.md) | 常见问题汇总 |
| [hardware/](hardware/) | 开发板原理图 |

## 例程

| 例程 | 功能 | 依赖 |
| --- | --- | --- |
| [wifi_cmd](components/p4_wireless/examples/wifi_cmd/) | 串口命令行：扫描、连接、SoftAP、ping、iperf | p4_wireless、iperf |
| [blufi](components/p4_wireless/examples/blufi/) | 手机通过 BLE 给板子配网 | p4_wireless |
| [lcd_lvgl](examples/lcd_lvgl/) | 4.96 寸 MIPI-DSI 屏 + 触摸，运行 LVGL widgets demo | 板级定义、LVGL |
| [camera](examples/camera/) | GC4023 摄像头 1080P 实时预览到屏幕 | 板级定义、esp_video |
| [audio](examples/audio/) | 播放提示音，并把麦克风声音实时回放到喇叭 | 板级定义、esp_codec_dev |
| [sdcard](examples/sdcard/) | 挂载 TF 卡，读写文件并列出目录 | 板级定义 |
| [usb_msc](examples/usb_msc/) | USB 主机读写 U 盘，测试读写速度 | usb_host_msc |
| [webrtc](examples/webrtc/) | 浏览器直连板子：摄像头视频 + 双向语音 | p4_wireless、板级定义、esp-webrtc-solution |

外部依赖由 ESP-IDF 组件管理器在首次编译时自动下载，需要联网。

## 组件

| 组件 | 内容 |
| --- | --- |
| [p4_wireless](components/p4_wireless/) | 模组 WiFi / BLE 组件（预编译） |
| [espp4_a_boards](components/espp4_a_boards/) | 开发板的 esp_board_manager 板级定义（屏、触摸、摄像头、音频、TF 卡） |
| [gc4023](components/gc4023/) | GC4023 摄像头的 esp_cam_sensor 驱动 |

## 已验证环境

- 开发板：ESP32-P4 DevKit v1.1
- ESP-IDF v5.5.3（全部例程）；WiFi / BLE 例程另在 v6.0.1 验证

## 许可证

本仓库采用 [Apache License 2.0](LICENSE)，例外和第三方内容见 [NOTICE](NOTICE)。
其中 `components/p4_wireless` 预编译库为专有软件，可免费使用和随产品再分发，仅限在 ESPP4-A 模组上使用，详见 [其许可协议](components/p4_wireless/LICENSE)。
