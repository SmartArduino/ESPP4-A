# webrtc：浏览器音视频通话

开发板通过 GC4023 向浏览器发送 H.264 视频，通过 ES8311 与浏览器进行双向音频通话。板端视频配置为 1920×1080、25fps，音频协商支持 OPUS 和 G711A。

摄像头、音频设备和 BOOT 按键由 [espp4_a_boards](../../components/espp4_a_boards/esp32_p4_devkit) 初始化，WiFi 由 [p4_wireless](../../components/p4_wireless) 提供。网页和 HTTPS 信令服务都运行在开发板上。

## 配置与构建

接好摄像头、麦克风和扬声器。电脑需要可用的摄像头、麦克风，以及支持 WebRTC 的浏览器；网页会同时请求摄像头和麦克风权限。

加载 ESP-IDF v5.5.3 的 `export.sh`，从 `ESPP4-A` 目录执行：

```bash
cd examples/webrtc
idf.py set-target esp32p4
idf.py menuconfig
```

在 `Example Configuration` 菜单中设置 `EXAMPLE_WIFI_SSID` 和 `EXAMPLE_WIFI_PASSWORD`，然后构建、烧录，串口按实际设备修改：

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建需联网下载依赖。工程已包含 `components/gen_bmgr_codes`，并通过相对路径引用仓库内的板级组件、GC4023 和无线组件，保留仓库目录结构即可构建。

## 发起通话

1. 将电脑和开发板接入同一局域网，等待串口输出 `got ip:` 和网页地址。
2. 在浏览器打开 `https://<开发板IP>/webrtc/test`。工程使用内置测试证书，若浏览器提示证书不受信任，需先按浏览器方式信任该证书。
3. 点击 `Call`，允许网页访问摄像头和麦克风。连接成功后，串口输出 `call established`，网页的 `Remote Video` 显示开发板画面。
4. 对任一端的麦克风说话，检查另一端的音频播放。点击 `Hang Up` 结束通话。

打开网页后也可按开发板 BOOT 键发送 `RING`，网页收到后播放铃声，再点击 `Call` 接通。铃声播放受浏览器的自动播放策略影响。

## 当前范围

- 板端只向浏览器发送视频，接收并播放浏览器音频；网页的 `Local Video` 是电脑本地预览。
- `Open Door` 只发送 `OPEN_DOOR` 并收到 `DOOR_OPENED` 确认消息，没有控制门锁 GPIO。
- 页面保留检测事件列表，板端未实现行人检测；`RTP Transformer` 默认关闭，普通通话保持关闭。
- HTTPS 信令同时只允许一个 SSE 监听连接，测试时只打开一个页面。默认没有配置 TURN，先在同一局域网验证。

通话和按键处理见 [webrtc.c](main/webrtc.c)，音视频初始化见 [media_sys.c](main/media_sys.c)，网页见 [webrtc_test.html](main/webrtc_test.html)。
