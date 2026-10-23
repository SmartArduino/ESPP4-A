# lcd_lvgl：屏幕与触摸

在 ST7102 屏幕上运行 LVGL 9 的 `lv_demo_widgets()`，用 ST7123 触摸驱动处理点击和拖动。

屏幕、触摸和背光由 [espp4_a_boards](../../components/espp4_a_boards/esp32_p4_devkit) 初始化。ST7102 的触摸部分使用兼容的 ST7123 协议。

## 准备与运行

使用 ESPP4-A，接好配套屏幕及触摸连接。加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行，串口按实际设备修改：

```bash
cd examples/lcd_lvgl
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。工程已包含 `components/gen_bmgr_codes`，保留仓库目录结构即可构建。

## 运行结果

串口输出 `LVGL widgets demo started` 后，屏幕显示 LVGL 控件演示界面，可以点击和拖动操作。
