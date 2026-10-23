# audio：音频播放与录音

使用 ES8311 播放和录音：程序先播放 2 秒 1kHz 提示音，再把麦克风声音实时回放到扬声器。

音频设备由 [espp4_a_boards](../../components/espp4_a_boards/esp32_p4_devkit) 初始化，应用通过板级句柄读写音频，不单独配置引脚。

## 准备与运行

使用 ESPP4-A，接好麦克风和扬声器。加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行，串口按实际设备修改：

```bash
cd examples/audio
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。工程已包含 `components/gen_bmgr_codes`，保留仓库目录结构即可构建。

## 运行结果

串口输出 `ES8311 ready` 后喇叭响 2 秒 1kHz 提示音；随后输出 `mic -> speaker loopback`，对着麦克风说话能从喇叭听到。
