# usb_msc：USB 读写 U 盘

ESP32-P4 作为 USB 主机识别 U 盘，挂载后列出根目录，写入一个 32MB 的测试文件再读回，打印读写速度，最后删除测试文件。拔出 U 盘后自动卸载，可以反复插拔。

使用 ESP32-P4 的 USB 2.0 高速 OTG 接口，驱动为乐鑫的 [usb_host_msc](https://components.espressif.com/components/espressif/usb_host_msc) 组件，挂载路径为 `/usb`，文件操作使用标准 C 接口（`open` / `write` / `read` / `opendir`）。

## 准备与运行

使用 ESPP4-A，通过 Type-C 转 USB-A 的 OTG 转接头，把 U 盘接到开发板的 USB 口（不是 UART-DEBUG 口）。U 盘需为 MBR 分区表、FAT32 格式；挂载失败不会自动格式化，盘上原有文件不受影响。

加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行，串口按实际设备修改：

```bash
cd examples/usb_msc
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。

## 运行结果

串口先输出 `Waiting for a USB storage device`。插入 U 盘后依次打印 VID/PID、型号和容量，`Mounted at /usb`，根目录文件列表，以及 `Write ... MB/s`、`Read ... MB/s` 两行读写速度。拔出时输出 `Device disconnected`。U 盘格式不支持时输出 `Mount failed`。
