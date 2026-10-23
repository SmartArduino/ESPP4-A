[中文](README.md) | English

# blufi: WiFi Provisioning over Bluetooth

Send the WiFi name and password from a phone over the BluFi protocol so the ESPP4-A connects to a router. The WiFi and BLE controller are provided by [p4_wireless](../..), and the Bluetooth host is ESP-IDF NimBLE.

This project builds the official example in `$IDF_PATH/examples/bluetooth/blufi/main` directly, so its behavior follows the ESP-IDF version you use.

## Build and Run

With the ESP-IDF environment (`export.sh`) loaded, run in this example's directory, using your actual serial port:

```bash
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Keep NimBLE, `CONFIG_BT_CONTROLLER_DISABLED=y` and the disabled UART transport in [sdkconfig.defaults](sdkconfig.defaults); HCI is handled by `p4_wireless`.

## Provision from a Phone

1. Install the EspBlufi app, turn on Bluetooth and grant scan permission.
2. Wait for `BLUFI init finish` in the log, then scan and connect in the app. With ESP-IDF v5.5.3 the default advertising name is `BLUFI_DEVICE`.
3. Choose STA provisioning, send the router SSID and password, then connect.
4. Check the WiFi result in the app; the log shows BluFi and WiFi events.

No WiFi credentials are needed at build time. With other ESP-IDF versions, the device name and options follow that version's official example. [CMakeLists.txt](CMakeLists.txt) defines `CONFIG_SOC_BLUFI_SUPPORTED` so the official BluFi app builds on ESP32-P4.
