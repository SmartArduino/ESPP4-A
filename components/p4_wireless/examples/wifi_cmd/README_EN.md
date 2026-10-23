[中文](README.md) | English

# wifi_cmd: WiFi Console

Scan for access points, connect to WiFi, start a SoftAP, and check connectivity and throughput with ping and iperf from a serial console. Wireless is provided by [p4_wireless](../..).

## Build and Run

Use an ESPP4-A board. With the ESP-IDF environment (`export.sh`) loaded, run in this example's directory, using your actual serial port:

```bash
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

The first build downloads the `espressif/iperf` dependency. After boot, type the commands below at the `wifi>` prompt.

## Commands

`<>` is required and `[]` is optional; do not type the brackets. Use double quotes for names or passwords with spaces.

| Command | Description |
| --- | --- |
| `help` | Show help |
| `scan` | Scan nearby access points |
| `sta <ssid> [password]` | Connect to a router, omit the password for an open network |
| `disconnect` | Disconnect |
| `ap <ssid> [password] [channel]` | Start a SoftAP, omit the password for an open network |
| `stop` | Stop WiFi |
| `info` | Show mode, MAC, IP, firmware version and PHY rate |
| `ping <host> [count]` | Ping an IP or host name, 4 times by default |
| `ps <mode>` | Power save mode: `none`, `min` or `max` |
| `iperf -a` | Stop the running iperf |

Connect to a router and check the data path (replace the name, password and address with your own):

```text
scan
sta "myssid" "mypassword"
info
ping 192.168.1.1 4
```

Wait for `got ip:` in the log before running ping or iperf. If the connection fails or drops, run `sta` again. STA and SoftAP are switched by command; only one mode runs at a time.

Start a SoftAP with a password:

```text
ap "P4-Test" "12345678" 6
info
```

## Measure Throughput with iperf

Use iperf 2.x on the peer; it is not compatible with iperf3. Put the PC and the board on the same LAN, then start the server on the PC:

```bash
iperf -s -i 1
```

At the board's `wifi>` prompt, replacing the address with the PC's IPv4 address:

```text
ps none
iperf -c 192.168.1.100 -t 10 -i 1
```

You can also run `iperf -s` on the board and `iperf -c <board IP> -t 10 -i 1` on the PC. Only one iperf can run at a time.

| iperf option | Meaning |
| --- | --- |
| `-s` / `-c <IP>` | Server / client, pick one |
| `-u` | UDP, TCP by default |
| `-t <sec>` | Duration |
| `-i <sec>` | Report interval |
| `-p <port>` | Port, same on both sides |
| `-l <bytes>` | Send buffer length |
| `-b <Mbit/s>` | Bandwidth limit |

See [main/main.c](main/main.c) for the code. [sdkconfig.defaults](sdkconfig.defaults) sets the TCP buffers and task parameters needed for high throughput and can be reused in your own project.
