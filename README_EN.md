[中文](README.md) | English

# ESPP4-A

ESPP4-A is a module based on Espressif **ESP32-P4**. ESP32-P4 has no radio of its own, so the module integrates a wireless chip that provides **WiFi (2.4 / 5 GHz) + BLE**.
Applications use the standard ESP-IDF `esp_wifi`, `esp_netif` and NimBLE APIs, the same way as on-chip WiFi on other ESP32 chips.

This repository contains module and board documentation, a setup guide, and examples for each peripheral. Each example's README explains how to use that peripheral.

## Quick Start

```sh
source ~/esp/esp-idf/export.sh      # your ESP-IDF path, v5.5 or later
cd components/p4_wireless/examples/wifi_cmd
idf.py set-target esp32p4
idf.py -p /dev/ttyUSB0 flash monitor
```

At the `wifi>` prompt, type `sta "your-ssid" "your-password"`. When the log shows `got ip:`, the module and WiFi are working. New to ESP-IDF? See the [setup guide](docs/03_开发环境搭建.md) (Chinese).

## Why ESPP4-A

Compared with the common ESP32-P4 + ESP32-C5 / C6 ([ESP-Hosted](https://github.com/espressif/esp-hosted-mcu)) two-chip design:

- **One module**: ESP32-P4 and the wireless chip are connected by SDIO inside the module. No second chip on your PCB and no SDIO / reset / wake-up routing.
- **One firmware**: the wireless firmware is stored in the ESP32-P4 flash and loaded by the driver in `esp_wifi_init()`. Nothing to flash on the wireless chip separately, and it is updated together with your application by OTA.
- **Higher throughput** (5 GHz, SDIO 4-bit 40 MHz, Mbit/s):

| Direction            |        ESPP4-A | ESP32-P4 + ESP32-C5 |
| -------------------- | -------------: | ------------------: |
| TCP TX (board → PC) | **42.7** |                  23 |
| TCP RX (PC → board) | **40.1** |                  32 |
| UDP TX (board → PC) | **76.0** |                  67 |
| UDP RX (PC → board) | **85.4** |                  68 |

ESPP4-A numbers were measured with the `wifi_cmd` example and iperf 2, with the PC wired to the router. ESP32-P4 + ESP32-C5 numbers are Espressif's published [ESP-Hosted results](https://github.com/espressif/esp-hosted-mcu/blob/main/docs/design/performance.md). The two were measured with different routers and RF environments, so compare them as orders of magnitude.

## Documentation (Chinese)

| Document                                  | Contents                                                      |
| ----------------------------------------- | ------------------------------------------------------------- |
| [01_模组介绍](docs/01_模组介绍.md)         | Module specs, advantages, pinout, pins used inside the module |
| [02_开发板介绍](docs/02_开发板介绍.md)     | DevKit v1.1 on-board peripherals and GPIO assignment          |
| [03_开发环境搭建](docs/03_开发环境搭建.md) | Install ESP-IDF, build, flash, monitor                        |
| [04_常见问题](docs/04_常见问题.md)         | FAQ                                                           |
| [hardware/](hardware/)                     | DevKit schematic                                              |

## Examples

| Example                                                          | Description                                                     | Depends on                                         |
| ---------------------------------------------------------------- | --------------------------------------------------------------- | -------------------------------------------------- |
| [wifi_cmd](components/p4_wireless/examples/wifi_cmd/README_EN.md) | Serial console: scan, connect, SoftAP, ping, iperf              | p4_wireless, iperf                                 |
| [blufi](components/p4_wireless/examples/blufi/README_EN.md)       | WiFi provisioning from a phone over BLE                         | p4_wireless                                        |
| [lcd_lvgl](examples/lcd_lvgl/)                                    | 4.96" MIPI-DSI display + touch running the LVGL widgets demo    | board definition, LVGL                             |
| [camera](examples/camera/)                                        | GC4023 camera 1080P live preview on the display                 | board definition, esp_video                        |
| [audio](examples/audio/)                                          | Play a tone, then loop the microphone back to the speaker       | board definition, esp_codec_dev                    |
| [sdcard](examples/sdcard/)                                        | Mount a TF card, read and write files, list the directory       | board definition                                   |
| [usb_msc](examples/usb_msc/)                                      | USB host reads and writes a USB flash drive, measures the speed | usb_host_msc                                       |
| [webrtc](examples/webrtc/)                                        | Browser-to-board WebRTC: camera video + two-way audio           | p4_wireless, board definition, esp-webrtc-solution |

External dependencies are downloaded by the ESP-IDF Component Manager on the first build, so an internet connection is required.

## Components

| Component                                         | Contents                                                                                   |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| [p4_wireless](components/p4_wireless/README_EN.md) | WiFi / BLE component for the module (prebuilt)                                             |
| [espp4_a_boards](components/espp4_a_boards/) | esp_board_manager board definition for the DevKit (display, touch, camera, audio, TF card) |
| [gc4023](components/gc4023/)                       | esp_cam_sensor driver for the GC4023 camera                                                |

## Tested With

- Board: ESP32-P4 DevKit v1.1
- ESP-IDF v5.5.3 (all examples); WiFi / BLE examples also on v6.0.1

## License

This repository is licensed under the [Apache License 2.0](LICENSE). See [NOTICE](NOTICE) for exceptions and third-party content.
The prebuilt `components/p4_wireless` library is proprietary: it is free to use and to redistribute with your products, on ESPP4-A modules only. See [its license](components/p4_wireless/LICENSE).
