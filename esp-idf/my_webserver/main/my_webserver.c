#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "driver/uart.h"

static const char *TAG = "webserver";

// UART configuration defines for UART1 on GPIO16 (RX) and GPIO17 (TX)
#define UART_PORT_NUM   UART_NUM_1
#define UART_TX_PIN     17
#define UART_RX_PIN     16
#define UART_BUF_SIZE   1024

/********************** UART FUNCTIONS ***************************/

void init_uart(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_LOGI(TAG, "UART initialized on TX:%d RX:%d", UART_TX_PIN, UART_RX_PIN);
}

void uart_task(void *pvParameters) {
    uint8_t data[UART_BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_PORT_NUM, data, UART_BUF_SIZE, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Received over UART: %s", (char *)data);
            // Echo the received data back over UART
            uart_write_bytes(UART_PORT_NUM, (const char *)data, len);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/********************** SPIFFS AND HTTP SERVER FUNCTIONS ***************************/

void init_spiffs(void) {
    ESP_LOGI(TAG, "Mounting SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error mounting SPIFFS: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS mounted");
    }
}

esp_err_t root_get_handler(httpd_req_t *req) {
    FILE *f = fopen("/spiffs/index.html", "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open /spiffs/index.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "text/html");
    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t connect_post_handler(httpd_req_t *req) {
    char buf[200];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    char ssid[64] = {0};
    sscanf(buf, "{\"ssid\":\"%63[^\"]", ssid);

    const char *pass = "monmotdepasse";

    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Saving: ssid='%s', pass='%s'", ssid, pass);
    nvs_set_str(nvs, "ssid", ssid);
    nvs_set_str(nvs, "pass", pass);
    nvs_commit(nvs);
    nvs_close(nvs);

    httpd_resp_sendstr(req, "OK");

    ESP_LOGI(TAG, "Config saved, restarting in 1s...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

#define SSDP_PORT 1900
#define SSDP_MCAST_IP "239.255.255.250"
#define SSDP_MSG "M-SEARCH * HTTP/1.1\r\n" \
                 "HOST: 239.255.255.250:1900\r\n" \
                 "MAN: \"ssdp:discover\"\r\n" \
                 "MX: 2\r\n" \
                 "ST: ssdp:all\r\n\r\n"

char hue_bridge_ip[32] = {0};

void discover_hue_bridge() {
    ESP_LOGI(TAG, "Starting SSDP discovery for Hue Bridge...");

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Socket error: %d", errno);
        return;
    }

    struct ip_mreq mreq = {
        .imr_multiaddr.s_addr = inet_addr(SSDP_MCAST_IP),
        .imr_interface.s_addr = htonl(INADDR_ANY)
    };
    setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

    struct timeval timeout = { .tv_sec = 3, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SSDP_PORT),
        .sin_addr.s_addr = inet_addr(SSDP_MCAST_IP)
    };

    sendto(sock, SSDP_MSG, strlen(SSDP_MSG), 0,
           (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    char rx_buffer[1024];
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);
        if (len < 0) break;

        rx_buffer[len] = 0;
        ESP_LOGI(TAG, "SSDP: %s", rx_buffer);

        if (strstr(rx_buffer, "IpBridge") ||
            strstr(rx_buffer, "hue-bridgeid") ||
            strstr(rx_buffer, "Philips")) {
            strncpy(hue_bridge_ip, inet_ntoa(source_addr.sin_addr), sizeof(hue_bridge_ip) - 1);
            ESP_LOGI(TAG, "Hue Bridge found: %s", hue_bridge_ip);
            break;
        }
    }
    close(sock);

    if (strlen(hue_bridge_ip) == 0) {
        ESP_LOGW(TAG, "No Hue Bridge found");
    }
}

esp_err_t whoami_handler(httpd_req_t *req) {
    char ipstr[16] = {0};
    nvs_handle_t nvs;
    if (nvs_open("wifi", NVS_READONLY, &nvs) == ESP_OK) {
        size_t len = sizeof(ipstr);
        if (nvs_get_str(nvs, "last_ip", ipstr, &len) == ESP_OK) {
            char json[64];
            snprintf(json, sizeof(json), "{\"ip\":\"%s\"}", ipstr);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr(req, json);
            nvs_close(nvs);
            return ESP_OK;
        }
        nvs_close(nvs);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

esp_err_t find_hubs_handler(httpd_req_t *req) {
    discover_hue_bridge();
    if (strlen(hue_bridge_ip) > 0) {
        char json[64];
        snprintf(json, sizeof(json), "{\"ip\":\"%s\"}", hue_bridge_ip);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, json);
    } else {
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

esp_err_t connect_hue_handler(httpd_req_t *req) {
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = 0;

    char post_data[] = "{\"devicetype\": \"esp_hue#user\"}";
    char url[128];
    snprintf(url, sizeof(url), "http://%s/api", hue_bridge_ip);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        char response[256] = {0};
        esp_http_client_read_response(client, response, sizeof(response) - 1);
        ESP_LOGI(TAG, "Hue Response: %s", response);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response);
    } else {
        ESP_LOGE(TAG, "Error connecting to Hue: %s", esp_err_to_name(err));
        httpd_resp_send_500(req);
    }
    esp_http_client_cleanup(client);
    return ESP_OK;
}

httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
httpd_uri_t connect_uri = { .uri = "/connect", .method = HTTP_POST, .handler = connect_post_handler };
httpd_uri_t find_hubs = { .uri = "/find_hubs", .method = HTTP_GET, .handler = find_hubs_handler };
httpd_uri_t connect_hue = { .uri = "/connect_hue", .method = HTTP_POST, .handler = connect_hue_handler };
httpd_uri_t whoami_uri = { .uri = "/whoami", .method = HTTP_GET, .handler = whoami_handler };

httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &connect_uri);
        httpd_register_uri_handler(server, &find_hubs);
        httpd_register_uri_handler(server, &connect_hue);
        httpd_register_uri_handler(server, &whoami_uri);
        ESP_LOGI(TAG, "Web server started on http://192.168.4.1");
    } else {
        ESP_LOGE(TAG, "Error starting webserver");
    }
    return server;
}

/********************** WIFI FUNCTIONS ***************************/

void start_ap_mode(void) {
    wifi_config_t wifi_config = { .ap = {
        .ssid = "ESP_Arty",
        .ssid_len = strlen("ESP_Arty"),
        .password = "12345678",
        .max_connection = 4,
        .authmode = WIFI_AUTH_WPA_WPA2_PSK
    } };

    if (strlen("12345678") == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    start_webserver();
    ESP_LOGI(TAG, "AP mode activated: ESP_Arty");
}

void start_sta_mode(const char *ssid, const char *pass) {
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "STA connecting to SSID: '%s'", ssid);
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));

        nvs_handle_t nvs;
        if (nvs_open("wifi", NVS_READWRITE, &nvs) == ESP_OK) {
            char ipstr[16];
            sprintf(ipstr, IPSTR, IP2STR(&event->ip_info.ip));
            nvs_set_str(nvs, "last_ip", ipstr);
            nvs_commit(nvs);
            nvs_close(nvs);
        }
    }
}

/********************** MAIN APPLICATION ***************************/

void app_main(void) {
    // Initialize NVS flash
    nvs_flash_init();
    // Initialize SPIFFS for serving files
    init_spiffs();
    // Initialize TCP/IP network interface
    esp_netif_init();
    // Create default event loop
    esp_event_loop_create_default();

    // Register WiFi and IP event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Initialize UART on GPIO16 (RX) and GPIO17 (TX)
    init_uart();
    // Create a task to handle UART data
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);

    // Retrieve WiFi configuration from NVS
    char ssid[64] = "", pass[64] = "";
    nvs_handle_t nvs;
    nvs_open("wifi", NVS_READONLY, &nvs);
    size_t len = 64;
    esp_err_t err1 = nvs_get_str(nvs, "ssid", ssid, &len);
    len = 64;
    esp_err_t err2 = nvs_get_str(nvs, "pass", pass, &len);
    nvs_close(nvs);

    ESP_LOGI(TAG, "NVS read: ssid='%s' (%s), pass='%s' (%s)", ssid,
             esp_err_to_name(err1), pass, esp_err_to_name(err2));

    // Decide AP or STA mode based on stored credentials
    if (strlen(ssid) > 0 && strlen(pass) > 0) {
        start_sta_mode(ssid, pass);
        vTaskDelay(pdMS_TO_TICKS(3000));
        start_webserver();
    } else {
        start_ap_mode();
    }
}
