# Universal HQ for IOT Hubs - UART Protocol
---

## ESP32 Zigbee Hub Discovery & UART Interface

This project implements a Wi-Fi-capable HTTP server on the ESP32 with support for:

- Web-based Wi-Fi setup (AP and STA modes)
- SSDP-based auto-discovery of Philips Hue Bridges
- UART communication with a SparkFun nRF52840 Mini Pro (Zigbee Coordinator)
- Persistent config via NVS
- REST API for integration with external systems
- SPIFFS-based frontend serving
- Logging and echoing of UART traffic

---

## Architecture Overview

```

+-------------+     UART      +-------------+      Wi-Fi      +------------+
\| nRF52840 ZC | ------------> |   ESP32     | --------------> | Hue Bridge |
\| (Zigbee ZC) |   (GPIO 16/17)| (Web + API) |   HTTP REST     | via IP     |
+-------------+               +-------------+                 +------------+

```

---

## Features Summary

### Wi-Fi Setup

- **AP mode** (SSID: `ESP_Arty`) is started by default if no Wi-Fi credentials are saved.
- Users can submit SSID via a web form (POST to `/connect`).
- After restart, the ESP32 uses **STA mode** to connect to the saved Wi-Fi network.

### REST API Endpoints

| Endpoint        | Method | Description                                  |
|-----------------|--------|----------------------------------------------|
| `/`             | GET    | Serves the main HTML UI from SPIFFS          |
| `/connect`      | POST   | Accepts SSID (stores in NVS and restarts)    |
| `/find_hubs`    | GET    | Discovers Hue Bridges via SSDP               |
| `/connect_hue`  | POST   | Sends pairing request to detected Hue bridge |
| `/whoami`       | GET    | Returns last known IP address (from NVS)     |

---

## UART Interface (with SparkFun nRF52840)

### UART Configuration

- **Port:** UART1  
- **TX:** GPIO17  
- **RX:** GPIO16  
- **Baud rate:** 115200  
- **Format:** 8N1 (8 bits, no parity, 1 stop bit)

### What the ESP32 Receives

The SparkFun nRF52840 acts as a Zigbee Coordinator and runs ZBOSS stack logic that:

1. Starts in Zigbee Coordinator mode
2. Sends a `Match_Desc_req` broadcast to find Zigbee Light Link devices
3. On receiving responses from ZLL devices (e.g. Philips Hue bulbs), it extracts their short NWK address
4. Transmits messages over UART to the ESP32, such as:

```

Device found: NWK=0x34FA
Device found: NWK=0xA2B1

```

### ESP32 Processing of UART

- A dedicated FreeRTOS task (`uart_task`) listens on UART1.
- When a message is received, it is:
  - Logged to the console using `ESP_LOGI`
  - Echoed back via UART (for debug or handshake confirmation)
- These messages can be parsed or exposed via API/web in future extensions.

### Sample UART Log Output

```

I (15710) webserver: Received over UART: Device found: NWK=0x34FA
I (16890) webserver: Received over UART: Device found: NWK=0xA2B1

```

---

## SPIFFS Frontend

- The file `/spiffs/index.html` is served via HTTP at `/`.
- SPIFFS is mounted on boot; if mounting fails, it auto-formats the partition.

---

## NVS Usage

- Stores:
  - `ssid` and `pass`: Wi-Fi credentials
  - `last_ip`: last known IP assigned in STA mode
- Configuration persists across reboots

---

## Workflow

1. Boot device: no config → starts AP mode (`ESP_Arty`)
2. Connect to AP, access `http://192.168.4.1`
3. Submit SSID via `/connect`
4. ESP32 reboots, joins network
5. Webserver launches, `/find_hubs` triggers Hue discovery
6. nRF52840 (via UART) sends back discovered Zigbee devices
7. ESP32 logs and can respond to UART messages

---

## Hardware Connections

| ESP32 Pin | Function        |
|-----------|-----------------|
| GPIO16    | UART RX (connect to TX of nRF52840) |
| GPIO17    | UART TX (connect to RX of nRF52840) |
| GND       | Common Ground   |

---

## Future Work

- Parse UART messages and store discovered Zigbee devices in RAM/NVS
- Enable REST API to return the list of Zigbee devices found
- Implement UART-to-Zigbee ON/OFF command relay
- Extend SSDP discovery to support other hubs (IKEA Tradfri, custom ones)

 