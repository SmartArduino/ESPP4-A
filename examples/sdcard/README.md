# sdcard：TF 卡文件读写

挂载 TF 卡，写入一个文本文件并读回，再列出根目录下的文件，最后卸载。文件操作使用标准 C 接口（`fopen` / `fprintf` / `fgets` / `opendir`）。

TF 卡设备由 [espp4_a_boards](../../components/espp4_a_boards/esp32_p4_devkit) 初始化，使用 SDMMC Slot 0、4bit 总线，挂载路径为 `/sdcard`。

## 准备与运行

使用 ESPP4-A，插入 FAT32 格式的 microSD 卡。程序会创建或覆盖 `/sdcard/hello.txt`；挂载失败不会自动格式化。

加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行，串口按实际设备修改：

```bash
cd examples/sdcard
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。工程已包含 `components/gen_bmgr_codes`，保留仓库目录结构即可构建。

## 运行结果

串口依次打印卡容量、`wrote /sdcard/hello.txt`、读回的内容 `Hello from ESPP4-A!` 和根目录文件列表，最后输出 `unmounted`。没插卡或格式不对时输出 `mount failed`。
