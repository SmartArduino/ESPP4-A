#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "ping/ping_sock.h"

#include "iperf.h"

#include "p4_wireless.h"

static const char *TAG = "main";

static esp_netif_t *s_sta_netif;
static esp_netif_t *s_ap_netif;

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data)
{
    (void)arg;
    (void)base;

    switch (id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "STA started");
        break;

    case WIFI_EVENT_STA_CONNECTED: {
        wifi_event_sta_connected_t *e = data;
        ESP_LOGI(TAG, "connected to \"%.*s\" ch=%d, waiting for DHCP...",
                 e->ssid_len, (const char *)e->ssid, e->channel);
        break;
    }

    case WIFI_EVENT_STA_DISCONNECTED: {
        wifi_event_sta_disconnected_t *e = data;
        ESP_LOGW(TAG, "disconnected, reason=%d", e->reason);
        break;
    }

    case WIFI_EVENT_SCAN_DONE: {
        wifi_event_sta_scan_done_t *e = data;
        ESP_LOGI(TAG, "scan done: status=%d, %d APs", (int)e->status, e->number);
        break;
    }

    case WIFI_EVENT_AP_START:
        ESP_LOGI(TAG, "SoftAP started");
        break;

    case WIFI_EVENT_AP_STOP:
        ESP_LOGI(TAG, "SoftAP stopped");
        break;

    case WIFI_EVENT_AP_STACONNECTED: {
        wifi_event_ap_staconnected_t *e = data;
        ESP_LOGI(TAG, "station joined: " MACSTR, MAC2STR(e->mac));
        break;
    }

    case WIFI_EVENT_AP_STADISCONNECTED: {
        wifi_event_ap_stadisconnected_t *e = data;
        ESP_LOGI(TAG, "station left: " MACSTR, MAC2STR(e->mac));
        break;
    }

    default:
        ESP_LOGD(TAG, "WIFI_EVENT id=%d", (int)id);
        break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t base,
                             int32_t id, void *data)
{
    (void)arg;
    (void)base;

    switch (id) {
    case IP_EVENT_STA_GOT_IP: {
        ip_event_got_ip_t *e = data;
        ESP_LOGI(TAG, "got ip: " IPSTR " gw " IPSTR " mask " IPSTR,
                 IP2STR(&e->ip_info.ip), IP2STR(&e->ip_info.gw),
                 IP2STR(&e->ip_info.netmask));
        break;
    }

    case IP_EVENT_STA_LOST_IP:
        ESP_LOGW(TAG, "IP lost");
        break;

    case IP_EVENT_AP_STAIPASSIGNED: {
        ip_event_ap_staipassigned_t *e = data;
        ESP_LOGI(TAG, "assigned IP to station: " IPSTR, IP2STR(&e->ip));
        break;
    }

    default:
        break;
    }
}

static int cmd_scan(int argc, char **argv)
{
    uint16_t num = 64;
    wifi_ap_record_t *records;
    esp_err_t err;

    (void)argc;
    (void)argv;

    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) == ESP_OK && mode != WIFI_MODE_STA) {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    printf("scanning...\n");
    err = esp_wifi_scan_start(NULL, true);
    if (err != ESP_OK) {
        printf("scan failed: %s\n", esp_err_to_name(err));
        return 1;
    }

    records = calloc(num, sizeof(*records));
    if (records == NULL) {
        return 1;
    }
    err = esp_wifi_scan_get_ap_records(&num, records);
    if (err != ESP_OK) {
        printf("get scan results failed: %s\n", esp_err_to_name(err));
        free(records);
        return 1;
    }

    printf("%-34s %-18s %4s %5s\n", "SSID", "BSSID", "CH", "RSSI");
    for (uint16_t i = 0; i < num; i++) {
        printf("%-34s " MACSTR " %4d %5d\n",
               (const char *)records[i].ssid, MAC2STR(records[i].bssid),
               records[i].primary, records[i].rssi);
    }
    free(records);
    return 0;
}

static int cmd_sta(int argc, char **argv)
{
    wifi_config_t cfg = {0};

    if (argc < 2) {
        printf("usage: sta <ssid> [password]\n");
        return 1;
    }

    strlcpy((char *)cfg.sta.ssid, argv[1], sizeof(cfg.sta.ssid));
    if (argc >= 3) {
        strlcpy((char *)cfg.sta.password, argv[2], sizeof(cfg.sta.password));
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    printf("connecting to \"%s\", see log for result\n", argv[1]);
    return 0;
}

static int cmd_disconnect(int argc, char **argv)
{
    esp_err_t err;

    (void)argc;
    (void)argv;

    err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        printf("disconnect failed: %s\n", esp_err_to_name(err));
        return 1;
    }
    return 0;
}

static int cmd_ap(int argc, char **argv)
{
    wifi_config_t cfg = {0};
    size_t pwd_len = 0;

    if (argc < 2) {
        printf("usage: ap <ssid> [password] [channel]\n");
        return 1;
    }

    if (s_ap_netif == NULL) {
        s_ap_netif = esp_netif_create_default_wifi_ap();
    }

    strlcpy((char *)cfg.ap.ssid, argv[1], sizeof(cfg.ap.ssid));
    cfg.ap.ssid_len = strlen((const char *)cfg.ap.ssid);

    if (argc >= 3) {
        strlcpy((char *)cfg.ap.password, argv[2], sizeof(cfg.ap.password));
        pwd_len = strlen((const char *)cfg.ap.password);
    }
    cfg.ap.authmode      = pwd_len ? WIFI_AUTH_WPA2_WPA3_PSK : WIFI_AUTH_OPEN;
    cfg.ap.channel       = (argc >= 4) ? (uint8_t)strtoul(argv[3], NULL, 10) : 0;
    cfg.ap.max_connection = 4;

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));

    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) {
        printf("SoftAP start failed: %s\n", esp_err_to_name(err));
        return 1;
    }
    printf("SoftAP \"%s\" started (%s)\n", argv[1], pwd_len ? "WPA2/WPA3" : "open");
    return 0;
}

static int cmd_stop(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    ESP_ERROR_CHECK(esp_wifi_stop());
    return 0;
}

static int cmd_info(int argc, char **argv)
{
    wifi_mode_t mode = WIFI_MODE_NULL;
    uint8_t mac[6] = {0};
    esp_netif_ip_info_t ip;
    char fw[64];

    (void)argc;
    (void)argv;

    esp_wifi_get_mode(&mode);
    printf("Mode      : %s\n",
           mode == WIFI_MODE_STA ? "STA" :
           mode == WIFI_MODE_AP  ? "AP"  : "NULL");

    if (esp_wifi_get_mac(WIFI_IF_STA, mac) == ESP_OK) {
        printf("MAC       : " MACSTR "\n", MAC2STR(mac));
    }

    if (mode == WIFI_MODE_STA) {
        wifi_ap_record_t ap;
        if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
            printf("AP        : %s (ch=%d, rssi=%d)\n", ap.ssid, ap.primary, ap.rssi);
            printf("BSSID     : " MACSTR "\n", MAC2STR(ap.bssid));
        } else {
            printf("AP        : not connected\n");
        }
        if (s_sta_netif && esp_netif_get_ip_info(s_sta_netif, &ip) == ESP_OK) {
            printf("IP        : " IPSTR "\n", IP2STR(&ip.ip));
        }
    } else if (mode == WIFI_MODE_AP) {
        wifi_sta_list_t list;
        if (esp_wifi_ap_get_sta_list(&list) == ESP_OK) {
            printf("Stations  : %d\n", list.num);
            for (int i = 0; i < list.num; i++) {
                printf("  " MACSTR " rssi=%d\n",
                       MAC2STR(list.sta[i].mac), list.sta[i].rssi);
            }
        }
        if (s_ap_netif && esp_netif_get_ip_info(s_ap_netif, &ip) == ESP_OK) {
            printf("AP IP     : " IPSTR "\n", IP2STR(&ip.ip));
        }
    }

    if (p4_wireless_get_fw_version(fw, sizeof(fw)) == ESP_OK) {
        printf("Firmware  : %s\n", fw);
    }

    {
        uint32_t tx_rate = 0, rx_rate = 0;
        if (p4_wireless_get_data_rate(&tx_rate, &rx_rate) == ESP_OK) {
            printf("PHY rate  : TX %lu bps\n", (unsigned long)tx_rate);
        }
    }
    return 0;
}

static SemaphoreHandle_t s_ping_done;

static void ping_on_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;
    uint16_t seqno;
    uint32_t size, elapsed;
    ip_addr_t target;

    (void)args;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target, sizeof(target));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &size, sizeof(size));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed, sizeof(elapsed));

    printf("%lu bytes from %s: icmp_seq=%u ttl=%u time=%lu ms\n",
           (unsigned long)size, ipaddr_ntoa(&target),
           (unsigned)seqno, (unsigned)ttl, (unsigned long)elapsed);
}

static void ping_on_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target;

    (void)args;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target, sizeof(target));
    printf("from %s icmp_seq=%u timeout\n", ipaddr_ntoa(&target), (unsigned)seqno);
}

static void ping_on_end(esp_ping_handle_t hdl, void *args)
{
    uint32_t sent, recved, duration;

    (void)args;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &sent, sizeof(sent));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &recved, sizeof(recved));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &duration, sizeof(duration));

    printf("%lu sent, %lu received, %lu%% loss, time %lu ms\n",
           (unsigned long)sent, (unsigned long)recved,
           (unsigned long)(sent ? (sent - recved) * 100 / sent : 100),
           (unsigned long)duration);
    xSemaphoreGive(s_ping_done);
}

static int cmd_ping(int argc, char **argv)
{
    struct addrinfo hint = { .ai_family = AF_INET };
    struct addrinfo *res = NULL;
    ip_addr_t target;
    esp_ping_config_t cfg = ESP_PING_DEFAULT_CONFIG();
    esp_ping_callbacks_t cbs = {
        .on_ping_success = ping_on_success,
        .on_ping_timeout = ping_on_timeout,
        .on_ping_end     = ping_on_end,
        .cb_args         = NULL,
    };
    esp_ping_handle_t ping = NULL;
    esp_err_t err;

    if (argc < 2) {
        printf("usage: ping <host|IP> [count]\n");
        return 1;
    }

    cfg.count = (argc >= 3) ? (uint32_t)strtoul(argv[2], NULL, 10) : 4;
    if (cfg.count == 0) {
        printf("count must be at least 1, the console waits until ping finishes\n");
        return 1;
    }

    if (getaddrinfo(argv[1], NULL, &hint, &res) != 0 || res == NULL) {
        printf("failed to resolve \"%s\", check with info that you are connected and have an IP\n", argv[1]);
        return 1;
    }
    memset(&target, 0, sizeof(target));
    inet_addr_to_ip4addr(ip_2_ip4(&target),
                         &((struct sockaddr_in *)res->ai_addr)->sin_addr);
    freeaddrinfo(res);

    cfg.target_addr = target;

    err = esp_ping_new_session(&cfg, &cbs, &ping);
    if (err != ESP_OK) {
        printf("create ping session failed: %s\n", esp_err_to_name(err));
        return 1;
    }

    printf("PING %s (%s) %lu times\n", argv[1], ipaddr_ntoa(&target),
           (unsigned long)cfg.count);

    xSemaphoreTake(s_ping_done, 0);   // Drop a late completion from a previous timed-out ping.
    esp_ping_start(ping);

    xSemaphoreTake(s_ping_done,
                   pdMS_TO_TICKS(cfg.count * (cfg.interval_ms + cfg.timeout_ms) + 2000));
    esp_ping_delete_session(ping);
    return 0;
}

static iperf_id_t s_iperf_id = -1;

static void iperf_state_cb(iperf_id_t id, iperf_state_data_t *data, void *priv)
{
    (void)priv;

    if (data->state == IPERF_CLOSED && id == s_iperf_id) {
        s_iperf_id = -1;
    }
}

static int cmd_iperf(int argc, char **argv)
{
    iperf_cfg_t cfg = {0};
    const char *host = NULL;
    uint16_t port = IPERF_DEFAULT_PORT;
    bool server = false, udp = false;

    cfg.format                = MBITS_PER_SEC;
    cfg.interval              = IPERF_DEFAULT_INTERVAL;
    cfg.time                  = IPERF_DEFAULT_TIME;
    cfg.bw_lim                = IPERF_DEFAULT_NO_BW_LIMIT;
    cfg.traffic_task_priority = IPERF_DEFAULT_TRAFFIC_TASK_PRIORITY;
    cfg.state_handler         = iperf_state_cb;

    for (int i = 1; i < argc; i++) {
        const char *opt = argv[i];
        const char *val = (i + 1 < argc) ? argv[i + 1] : NULL;

        if (!strcmp(opt, "-s")) {
            server = true;
        } else if (!strcmp(opt, "-u")) {
            udp = true;
        } else if (!strcmp(opt, "-a")) {
            if (s_iperf_id < 0) {
                printf("no iperf running\n");
                return 0;
            }
            iperf_stop_instance(s_iperf_id);
            s_iperf_id = -1;
            printf("stopped\n");
            return 0;
        } else if (val == NULL) {
            printf("option %s needs a value\n", opt);
            return 1;
        } else if (!strcmp(opt, "-c")) {
            host = val; i++;
        } else if (!strcmp(opt, "-i")) {
            cfg.interval = (uint32_t)strtoul(val, NULL, 10); i++;
        } else if (!strcmp(opt, "-t")) {
            cfg.time = (uint32_t)strtoul(val, NULL, 10); i++;
        } else if (!strcmp(opt, "-p")) {
            port = (uint16_t)strtoul(val, NULL, 10); i++;
        } else if (!strcmp(opt, "-l")) {
            cfg.len_send_buf = (uint16_t)strtoul(val, NULL, 10); i++;
        } else if (!strcmp(opt, "-b")) {
            cfg.bw_lim = (int32_t)(strtod(val, NULL) * 1000000);
            i++;
        } else {
            printf("unknown option: %s\n", opt);
            return 1;
        }
    }

    if (!server && host == NULL) {
        printf("usage: iperf -s [-u] [-i sec] [-t sec] [-p port]\n"
               "       iperf -c <IP> [-u] [-i sec] [-t sec] [-p port] [-l len] [-b Mbit/s]\n"
               "       iperf -a                      stop the running iperf\n"
               "Use iperf 2.x on the peer (not iperf3), e.g. iperf -s -i 3 on the PC\n");
        return 1;
    }
    if (server && host != NULL) {
        printf("use either -s or -c\n");
        return 1;
    }
    if (s_iperf_id >= 0) {
        printf("iperf already running, run iperf -a first\n");
        return 1;
    }

    if (cfg.interval == 0) {
        cfg.interval = 1;
    }
    if (cfg.time < cfg.interval) {
        cfg.time = cfg.interval;
    }

    if (server) {

        cfg.sport = port;
        cfg.flag  = IPERF_FLAG_SERVER | (udp ? IPERF_FLAG_UDP : IPERF_FLAG_TCP);
    } else {
        cfg.destination.type         = ESP_IPADDR_TYPE_V4;
        cfg.destination.u_addr.ip4.addr = esp_ip4addr_aton(host);
        if (cfg.destination.u_addr.ip4.addr == 0) {
            printf("invalid address: %s (IPv4 dotted decimal only, no DNS)\n", host);
            return 1;
        }
        cfg.dport = port;
        cfg.flag  = IPERF_FLAG_CLIENT | (udp ? IPERF_FLAG_UDP : IPERF_FLAG_TCP);
    }

    s_iperf_id = iperf_start_instance(&cfg);
    if (s_iperf_id < 0) {
        printf("iperf start failed: %s\n", strerror(errno));
        return 1;
    }

    printf("iperf %s %s started (id=%d, port %u, %lu s), stop with iperf -a\n",
           udp ? "UDP" : "TCP", server ? "server" : "client",
           (int)s_iperf_id, (unsigned)port, (unsigned long)cfg.time);
    return 0;
}

static int cmd_ps(int argc, char **argv)
{
    wifi_ps_type_t type;
    esp_err_t err;

    if (argc < 2) {
        printf("usage: ps <none|min|max>\n"
               "  none disables power save (for throughput); min/max save power but slow down downlink\n");
        return 1;
    }
    if (!strcmp(argv[1], "none")) {
        type = WIFI_PS_NONE;
    } else if (!strcmp(argv[1], "min")) {
        type = WIFI_PS_MIN_MODEM;
    } else if (!strcmp(argv[1], "max")) {
        type = WIFI_PS_MAX_MODEM;
    } else {
        printf("unknown value: %s (none/min/max)\n", argv[1]);
        return 1;
    }

    err = esp_wifi_set_ps(type);
    if (err != ESP_OK) {
        printf("set failed: %s\n", esp_err_to_name(err));
        return 1;
    }
    printf("power save -> %s\n", argv[1]);
    return 0;
}

static void register_commands(void)
{
    const esp_console_cmd_t cmds[] = {
        { .command = "scan",       .help = "Scan nearby APs",
          .hint = NULL,                        .func = &cmd_scan },
        { .command = "sta",        .help = "Connect to an AP in STA mode",
          .hint = "<ssid> [password]",         .func = &cmd_sta },
        { .command = "disconnect", .help = "Disconnect from the AP",
          .hint = NULL,                        .func = &cmd_disconnect },
        { .command = "ap",         .help = "Start SoftAP (open network without password)",
          .hint = "<ssid> [password] [channel]", .func = &cmd_ap },
        { .command = "stop",       .help = "Stop WiFi",
          .hint = NULL,                        .func = &cmd_stop },
        { .command = "info",       .help = "Show mode / MAC / IP / firmware version / PHY rate",
          .hint = NULL,                        .func = &cmd_info },
        { .command = "ping",       .help = "Ping a host",
          .hint = "<host|IP> [count]",         .func = &cmd_ping },
        { .command = "ps",         .help = "WiFi power save (set none before throughput tests)",
          .hint = "<none|min|max>",            .func = &cmd_ps },
        { .command = "iperf",      .help = "Measure throughput (peer uses iperf 2.x)",
          .hint = "-s | -c <IP> [-u] [-i s] [-t s] [-p port] [-b Mbit]",
          .func = &cmd_iperf },
    };

    s_ping_done = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(s_ping_done ? ESP_OK : ESP_ERR_NO_MEM);

    esp_console_register_help_command();
    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&cmds[i]));
    }
}

static void start_console(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

    repl_config.prompt = "wifi>";
    repl_config.task_stack_size = 8192;   // The default 4 KB stack is too small for printing scan results.

    register_commands();

    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESPP4-A WiFi console example");
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create the netif before esp_wifi_init(), otherwise no IP_EVENT is received.
    s_sta_netif = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed (%s)", esp_err_to_name(err));
        return;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "free heap: %lu bytes", (unsigned long)esp_get_free_heap_size());
    ESP_LOGI(TAG, "type help to list commands");

    start_console();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
