中文 | [English](README_EN.md)

# wifi_cmd：WiFi 命令行

通过串口命令扫描热点、连接 WiFi、开启 SoftAP，并用 ping、iperf 检查网络连通性和吞吐。无线通信由 [p4_wireless](../..) 提供。

## 构建与启动

使用 ESPP4-A。加载 ESP-IDF 环境（`export.sh`）后，在本例程目录下执行，串口按实际设备修改：

```bash
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

首次构建会下载 `espressif/iperf` 依赖。启动后出现 `wifi>` 提示符，在此输入下面的命令。

## 命令

`<>` 表示必填参数，`[]` 表示可选参数，输入时不带括号。名称或密码包含空格时使用双引号。

| 命令 | 作用 |
| --- | --- |
| `help` | 查看帮助 |
| `scan` | 扫描周围热点 |
| `sta <ssid> [password]` | 连接路由器，省略密码表示开放网络 |
| `disconnect` | 断开当前连接 |
| `ap <ssid> [password] [channel]` | 开启 SoftAP，省略密码表示开放网络 |
| `stop` | 停止 WiFi |
| `info` | 查看模式、MAC、IP、固件版本和可用的 PHY 速率 |
| `ping <host> [count]` | 测试 IP 或域名连通性，默认 4 次 |
| `ps <模式>` | 设置省电模式：`none`、`min` 或 `max` |
| `iperf -a` | 停止当前 iperf 测试 |

连接路由器并确认数据通路，名称、密码和地址按实际网络替换：

```text
scan
sta "myssid" "mypassword"
info
ping 192.168.1.1 4
```

等串口输出 `got ip:` 后再执行 ping 或 iperf。连接失败或断开后，重新执行 `sta`。STA 与 SoftAP 通过命令切换，同一时刻运行一种模式。

开启带密码的热点：

```text
ap "P4-Test" "12345678" 6
info
```

## 用 iperf 测吞吐

对端使用 iperf 2.x，协议与 iperf3 不兼容。先将电脑和开发板接入同一局域网，再在电脑终端启动服务端：

```bash
iperf -s -i 1
```

在开发板的 `wifi>` 提示符下输入，将地址替换为电脑的 IPv4 地址：

```text
ps none
iperf -c 192.168.1.100 -t 10 -i 1
```

也可以在开发板执行 `iperf -s`，电脑执行 `iperf -c <开发板IP> -t 10 -i 1`。同一时间只能运行一个 iperf 测试。

| iperf 参数 | 含义 |
| --- | --- |
| `-s` / `-c <IP>` | 服务端 / 客户端，二选一 |
| `-u` | 使用 UDP，默认 TCP |
| `-t <秒>` | 测试时长 |
| `-i <秒>` | 报告间隔 |
| `-p <端口>` | 测试端口，两端保持一致 |
| `-l <字节>` | 发送缓冲长度 |
| `-b <Mbit/s>` | 带宽限制 |

实现见 [main/main.c](main/main.c)。[sdkconfig.defaults](sdkconfig.defaults) 已配置高吞吐所需的 TCP 缓冲和任务参数，可直接用于自己的工程。
