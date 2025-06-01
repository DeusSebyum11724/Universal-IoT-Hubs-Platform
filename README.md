# **ESP32 | WROOM32 DevKit C** 

## Universal HQ for IOT Hubs

---

### Steps for running the web-server:

- Erasing the flash, because we implemented a function to save the hub's after connecting to them.
> `idf.py -p /dev/cu.SLAB_USBtoUART erase_flash`

- Flashing and monitoring using 
> `idf.py -p /dev/cu.SLAB_USBtoUART flash monitor`

- From the web interface, we press the `DISCOVER` button to find any hub in the network
(for now there are implemented functions for Hue and Tradfri with further development being taken in the considerations)
```
<h1>ESP32 Smart Hub</h1>
  <button onclick="discover()">Discover</button>
  <button onclick="pair()">Pair</button>
  <button onclick="listDevices()">List Devices</button>
  <div id="devices"></div>
  <div id="log">[LOG]</div>
```

> Functions in C:

| Function            | Purpose                                         |
| ------------------- | ----------------------------------------------- |
| `hue_discover()`    | Finds the Hue hub on the network                |
| `hue_pair()`        | Registers as an application (with `username`)   |
| `hue_get_devices()` | Retrieves the list of bulbs                     |
| `hue_set_state()`   | Controls a bulb                                 |
| `hue_get_info()`    | Information about the hub                       |

- We configured the driver (`hue.c`):
```
// hue.h

// Funcție care descoperă bridge-ul Hue folosind SSDP.
// Returnează true dacă a găsit, iar IP-ul este pus în ip_out (max 64 bytes).
bool hue_discover(char* ip_out);

// Funcție care face pairing cu bridge-ul Hue și obține un username.
// Returnează true dacă pairing-ul a reușit, username-ul este pus în user_out.
bool hue_pair(const char* ip, char* user_out);

// Returnează lista de dispozitive Hue sub formă de JSON, în json_out.
// Returnează true dacă cererea a avut succes.
bool hue_get_devices(const char* ip, const char* user, char* json_out, size_t max_len);

// Trimite o comandă de schimbare stare (on/off, bri etc.) pentru un device dat.
// Returnează true dacă succes.
bool hue_set_state(const char* ip, const char* user, const char* device_id, const char* state_json);

// Returnează informații despre bridge (model, MAC, versiune) sub formă JSON.
bool hue_get_info(const char* ip, const char* user, char* json_out, size_t max_len);

// Driverul Hue complet, expus pentru a fi folosit într-un array de drivere sau ca pointer activ.
#include "hub_interface.h"
extern smart_hub_driver_t hue_driver;
```

> Handlers:

```
static esp_err_t discover_handler(httpd_req_t *req) {
    if (current_driver->discover(hub_ip)) {
        ESP_LOGI(TAG, "Hub găsit la %s", hub_ip);
        return httpd_resp_send(req, hub_ip, HTTPD_RESP_USE_STRLEN);
    } else {
        return httpd_resp_send(req, "Hub not found", HTTPD_RESP_USE_STRLEN);
    }
}

static esp_err_t pair_handler(httpd_req_t *req) {
    if (!strlen(hub_ip)) return httpd_resp_send(req, "IP missing", HTTPD_RESP_USE_STRLEN);
    if (current_driver->pair(hub_ip, hub_user)) {
        ESP_LOGI(TAG, "Pairing OK. User = %s", hub_user);
        return httpd_resp_send(req, hub_user, HTTPD_RESP_USE_STRLEN);
    } else {
        return httpd_resp_send(req, "Pairing failed", HTTPD_RESP_USE_STRLEN);
    }
}

static esp_err_t list_handler(httpd_req_t *req) {
    static char json[4096];
    if (!strlen(hub_ip) || !strlen(hub_user)) return httpd_resp_send(req, "Missing IP/user", HTTPD_RESP_USE_STRLEN);
    if (current_driver->get_devices(hub_ip, hub_user, json, sizeof(json))) {
        return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    } else {
        return httpd_resp_send(req, "Device list failed", HTTPD_RESP_USE_STRLEN);
    }
}
```
> Serial Monitor after compiling and flashing:

```
I (707) phy_init: Saving new calibration data due to checksum failure or outdated calibration data, mode(2)
I (717) wifi:mode : sta (cc:db:a7:68:6f:f0)
I (717) wifi:enable tsf
I (1597) wifi:new:<6,1>, old:<1,0>, ap:<255,255>, sta:<6,1>, prof:1, snd_ch_cfg:0x0
I (1597) wifi:state: init -> auth (0xb0)
I (1607) wifi:state: auth -> assoc (0x0)
I (1617) wifi:state: assoc -> run (0x10)
I (1647) wifi:connected with Home_2.4G, aid = 8, channel 6, 40U, bssid = f0:55:01:2d:13:a8
I (1647) wifi:security: WPA2-PSK, phy: bgn, rssi: -65
I (1657) wifi:pm start, type: 1

I (1657) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (1667) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (2747) esp_netif_handlers: sta ip: 192.168.3.45, mask: 255.255.255.0, gw: 192.168.3.1
I (2747) smart-home: Webserver started cu toate rutele înregistrate!
I (2747) smart-home: ESP32 Smart Hub ready!
I (2747) main_task: Returned from app_main()
I (10777) wifi:<ba-add>idx:0 (ifx:0, f0:55:01:2d:13:a8), tid:0, ssn:1, winSize:64
W (18807) httpd_uri: httpd_uri: URI '/favicon.ico' not found
W (18807) httpd_txrx: httpd_resp_send_err: 404 Not Found - Nothing matches the given URI
I (21937) smart-home: Hub găsit la 192.168.3.59
I (29967) smart-home: Pairing OK. User = NPNs4xI1qw6dzkX39Agpm0WTiLQEI6JlPe-lGrgp
```
