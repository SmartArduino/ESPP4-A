中文 | [English](README_EN.md)

# blufi：通过蓝牙配置 WiFi

用手机通过 BluFi 协议发送 WiFi 名称和密码，让 ESPP4-A 连接路由器。WiFi 和 BLE 控制器由 [p4_wireless](../..) 提供，蓝牙 Host 使用 ESP-IDF 的 NimBLE。

工程直接编译当前 `$IDF_PATH/examples/bluetooth/blufi/main` 中的官方例程，因此应用行为随所用 ESP-IDF 版本变化。

## 构建与运行

加载 ESP-IDF 环境（`export.sh`）后，在本例程目录下执行，串口按实际设备修改：

```bash
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

保留 [sdkconfig.defaults](sdkconfig.defaults) 中的 NimBLE、`CONFIG_BT_CONTROLLER_DISABLED=y` 和 UART 传输禁用配置，HCI 通信由 `p4_wireless` 接管。

## 手机配网

1. 在手机上安装支持 BluFi 协议的 EspBlufi 应用，开启蓝牙并授予扫描权限。
2. 等串口输出 `BLUFI init finish`，在应用中扫描并连接设备。ESP-IDF v5.5.3 的默认广播名称为 `BLUFI_DEVICE`。
3. 选择 STA 配网，发送路由器的 SSID 和密码，再发起连接。
4. 在手机中查看 WiFi 连接结果，串口可查看 BluFi 连接和 WiFi 事件。

本例程无需在编译前写入 WiFi 账号。若修改 ESP-IDF 版本，设备名称和配网选项以对应版本的官方例程为准。[CMakeLists.txt](CMakeLists.txt) 已补充 `CONFIG_SOC_BLUFI_SUPPORTED`，用于在 P4 上编译官方 BluFi 应用。
