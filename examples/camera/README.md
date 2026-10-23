# camera：摄像头预览

采集 GC4023 图像，通过 PPA 缩放并旋转 90°，在 480×854 的 ST7102 屏幕上居中显示。

摄像头、屏幕和背光由 [espp4_a_boards](../../components/espp4_a_boards/esp32_p4_devkit) 初始化，GC4023 驱动位于 [components/gc4023](../../components/gc4023)。

## 准备与运行

使用 ESPP4-A，接好配套的 GC4023 摄像头和 ST7102 屏幕。加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行，串口按实际设备修改：

```bash
cd examples/camera
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。工程已包含 `components/gen_bmgr_codes`，保留仓库目录结构即可构建。

## 运行结果

串口依次输出 `LCD ready`、`Streaming on, 1920x1080`，屏幕显示摄像头实时画面，之后定期打印 `display fps`。
