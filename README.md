# Universal IoT Hub: Microprocessor for Protocol Convergence in Smart Networks


| | |
|-|-|
|`Author` | BARBIER Ileana Geneviève & BRIȘAN Andrei-Sebastian 

## Description
 ### Purpose:

Development of a hardware-software system based on two microprocessors (ESP32-WROOM32D + SparkFun Pro nRF52840 mini) that functions as a universal IoT gateway, capable of interconnecting and translating between various protocols: Matter, Zigbee, Z-Wave, BLE, Wi-Fi, Thread, etc.

### Objectives:
1. Design of a modular microprocessor/microcontroller architecture with support for protocol extension modules.
2. Hardware integration of modules:  Wi-Fi/BLE (ESP32-WROOM32D), Thread/Matter (nRF52840), Zigbee (e.g., CC2652), Z-Wave (ZGM130S).
3. Implementation of a software interface that:
Detects IoT devices in the network regardless of protocol, displays them in a web dashboard (hosted on the ESP), allows the creation of automation scenarios between devices across different protocols.
5. Development of a real-time monitoring WebUI.


## Motivation
The smart home ecosystem suffers from fragmentation due to multiple communication standards (Zigbee, Z-Wave, BLE, Thread, etc.), which limits interoperability between devices. This project aims to create a single hardware hub capable of bridging these technologies through a unified interface, enabling seamless integration and smarter automation scenarios.

## Architecture

### Block diagram

<!-- Make sure the path to the picture is correct -->
![Block Diagram](https://github.com/user-attachments/assets/df5a79a8-5ef0-4c6e-b556-020f0129d42c)
)

### Schematic

![Schematic](https://github.com/user-attachments/assets/3c3bc166-1eec-4644-9cd8-4a8fca808c11)
)

### Components


<!-- This is just an example, fill in with your actual components -->

| Device | Usage | Price |
|--------|--------|-------|
| ESP32-WROOM32D | Development board | [69,99 RON](https://www.optimusdigital.ro/en/wifi-boards/3053-placa-de-dezvoltare-esp32-cu-wifi-si-bluetooth.html?search_query=esp32&results=39) |
| Breadboard 300 points + 300 points | Project boards | 2 * [3,49RON](https://www.optimusdigital.ro/en/breadboards/13249-breadboard-300-puncte.html?search_query=breadboard&results=361)|
| Female-Female Wires| Connecting components | [6,99 RON](https://www.optimusdigital.ro/en/wires-with-connectors/652-10-cm-40p-female-female-wire.html?search_query=male+male&results=806&HTTP_REFERER=https%3A%2F%2Fwww.optimusdigital.ro%2Fen%2Fsearch%3Fcontroller%3Dsearch%26orderby%3Dposition%26orderway%3Ddesc%26search_query%3Dmale%2Bmale%26submit_search%3D) |
| Male-Female Wires| Connecting components | [5,99 RON](https://www.optimusdigital.ro/en/wires-with-connectors/653-10-cm-40p-male-to-female-wire.html?search_query=male+male+40p&results=38) |
| Male-Male Wires| Connecting components | [7,98 RON](https://www.optimusdigital.ro/en/wires-with-connectors/890-set-fire-tata-tata-40p-30-cm.html?search_query=male+male+40p&results=38) |
| 2 Pitch Male Pin Headers| Soldering on nRF52840 | [1,98 RON](https://www.optimusdigital.ro/en/pin-headers/464-colored-40p-254-mm-pitch-male-pin-header-red.html?search_query=Colored+40p+2.54+mm+Pitch+Male+Pin+Header+-+Red&results=4) |
| SparkFun Pro nRF52840| Thread/BLE/Matter| [229,14 RON](https://www.robofun.ro/wireless/placa-de-dezvoltare-sparkfun-pro-nrf52840-mini-cu-bluetooth.html) |

### Libraries

<!-- This is just an example, fill in the table with your actual components -->

| Library | Description | Usage |
|---------|-------------|-------|
| [libCoAP](https://github.com/obgm/libcoap) | A CoAP (RFC 7252) implementation in C | Used in ESP-IDF on ESP32-WROOM32D  to implement a CoAP server for communication with Thread/BLE devices ( nRF52840)  |
| [ESP-IDF](https://github.com/espressif/esp-idf) | Espressif IoT Development Framework. Official development framework for Espressif SoCs. | Provides hardware abstraction, drivers, FreeRTOS, and networking support including Wi-Fi  |
| [OpenThread](https://github.com/openthread/openthread) | OpenThread released by Google is an open-source implementation of the Thread networking protocol | Enables Thread protocol support on the nRF52840  |

### Software
ESP32-WROOM32D:
#### ESP-IDF extension in Visual Studio Code
SparkFun Pro nRF52840 Mini:
#### - GNU C/C++ Compiler (gcc-arm-none-eabi-10.3-2021.10-mac.tar.bz2) 
1. Downloaded into an accessible folder(ex. Downloads)
2. Unarchived and moved to /opt/arm-gcc
3. Provided high level permissions to the folder using % sudo xattr -rd com.apple.quarantine /opt/arm-gcc
#### - Command line tools and DFU Bootloader tools
1. nRF5x Command Line Tools for Mac, version 10.24.2, nrf-command-line-tools-10.24.2-Darwin.dmg
2. Installed adafruit-nrfutil-0.5.3 (pip3 install --user adafruit-nrfutil)
- Setting up the nRF5 SDK
1. Downloaded the Nordic's latest nRF5 SDK and extracted it to an accessible folder (ex. Downloads)
2. Modified the components/toolchains/gcc/Makefile.posix with the appropriate values: \
  `GNU_INSTALL_ROOT := /opt/arm-gcc/bin/` \
   `GNU_PREFIX := arm-none-eabi` \
   `GNU_VERSION := 10.3.1 `
3. Edited the board file [sparkfun_nrf52840_mini.h](https://github.com/sparkfun/nRF52840_Breakout_MDBT50Q/blob/master/Firmware/nRF5_SDK/components/boards/sparkfun_nrf52840_mini.h)
4. Placed that file in the components/boards directory
5. Modified components/boards/boards.h, adding the following before #elif defined(BOARD_CUSTOM): \
`#elif defined (BOARD_SPARKFUN_NRF52840_MINI)` \
`#include "sparkfun_nrf52840_mini.h"`
#### - Setting Up a VS Code Environment
1. Opened a Sample Folder in VS Code
2. In the explorer-prompt that opened, we navigated to the nRF5 SDK, then found examples/peripheral/blinky
3. Set Up c_cpp_properties.json \ editing configurations in main.c
```
{
    "env": {
        "nrfSDK": "C:\\nRF5\\nRF5_SDK_15.2.0_9412b96"
    },

    "configurations": [
        {
            "name": "ARMGCC",
            "includePath": [
                "${workspaceFolder}/",
                "${nrfSDK}/components/",
                "${nrfSDK}/components",
                "${nrfSDK}/components/libraries/experimental_memobj",
                "${nrfSDK}/components/libraries/experimental_section_vars",
                "${nrfSDK}/modules/nrfx/mdk",
                "${nrfSDK}/modules/nrfx/hal",
                "${nrfSDK}/components/libraries/balloc",
                "${nrfSDK}/components/libraries/experimental_log",
                "${nrfSDK}/components/libraries/experimental_log/src",
                "${nrfSDK}/components/libraries/delay",
                "${nrfSDK}/integration/nrfx",
                "${nrfSDK}/components/libraries/bsp",
                "${nrfSDK}/components/drivers_nrf/nrf_soc_nosd",
                "${nrfSDK}/components/libraries/strerror",
                "${nrfSDK}/components/boards",
                "${nrfSDK}/components/toolchain/cmsis/include",
                "${nrfSDK}/modules/nrfx",
                "${nrfSDK}/components/libraries/util",
                "${nrfSDK}/components/libraries/fifo",
                "${nrfSDK}/components/libraries/uart",
                "${nrfSDK}/integration/nrfx/legacy",
                "${nrfSDK}/components/libraries/delay",
                "${nrfSDK}/modules/nrfx/drivers/include",
                "${workspaceFolder}/sparkfun/blank/config",
                "${workspaceFolder}/sparkfun/blank"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE",
                "BOARD_SPARKFUN_NRF52840_MINI",
                "BSP_DEFINES_ONLY",
                "CONFIG_GPIO_AS_PINRESET",
                "FLOAT_ABI_HARD",
                "NRF52840_XXAA",
                "DCONFIG_NFCT_PINS_AS_GPIOS"
            ],
            "compilerPath": "C:\\Program Files (x86)\\GNU Tools ARM Embedded\\7 2018-q2-update\\bin\\arm-none-eabi-gcc.exe",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "clang-x64"
        }
    ],
    "version": 4
}
```
#### - Building Blinky for the SparkFun nRF52840 Mini
1. Copied and duplicated the pca10056 folder and renamed it to sparkfun_nrf52840_mini
2. Modified the board definitions (CFLAGS and ASMFLAGS definitions for -DBOARD_PCA10056) to -DBOARD_SPARKFUN_NRF52840_MINI:
3. Added DFU-Programming Targets
```
dfu-package: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
@echo Packaging $<
adafruit-nrfutil dfu genpkg --sd-req 0xFFFE --dev-type 0x0052 --application $<  _build/dfu-package.zip

bootload: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex dfu-package
@echo Flashing: $<
adafruit-nrfutil --verbose dfu serial --package _build/dfu-package.zip -p $(SERIAL_PORT) -b 115200 --singlebank --touch 1200
```
4. Modified the header of Makefile as:
```
PROJECT_NAME     := blinky_pca10056
TARGETS          := nrf52840_xxaa
OUTPUT_DIRECTORY := _build
NRFUTIL := /Users/asbri/Library/Python/3.13/bin/adafruit-nrfutil
SDK_ROOT := /Users/asbri/Downloads/DeviceDownload/nRF5_SDK_17.1.0_ddde560
PROJ_DIR := ../../..
```
5. Added at the bottom of the file :
```
dfu-package: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
@echo Packaging $<
$(NRFUTIL) dfu genpkg --sd-req 0xFFFE --dev-type 0x0052 --application $<  _build/dfu-package.zip

bootload: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex dfu-package
@echo Flashing: $<
$(NRFUTIL) --verbose dfu serial --package _build/dfu-package.zip -p $(SERIAL_PORT) -b 115200 --singlebank --touch 1200
```
#### - Revising the Linker Script's Memory Organization
1. Opened blinky_gcc_nrf52.ld and replaced the MEMORY section of the linker script with the below memory origin/length combinations:
```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x26000, LENGTH = 0xce000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x40000
}
```
2. Build and program in one swift command: \
`make bootload SERIAL_PORT=/dev/cu.usbmodem1101`

## Log

<!-- write every week your progress here -->

### Week 6 - 12 May
1. Finalized the component list and began sourcing hardware
2. Planned project architecture and started block diagram design

### Week 7 - 19 May
1. Initial testing of ESP32-WROOM32D  dev board and Wi-Fi web server setup
2. Breadboard layout and wiring test for ESP32 and nRF52840
3. Started the software part for ESP32-WROOM32D and partially for nRF52840

### Week 20 - 26 May
1. Finalized the software part for both ESP32-WROOM32D and Sparkfun Pro nRF52840 mini


## Reference links

<!-- Fill in with appropriate links and link titles -->
[UART, SPI, I2C Protocol basics](https://learn.sparkfun.com/tutorials/serial-communication/all)

[SparkFun nRF52840 Tutorial](https://learn.sparkfun.com/tutorials/sparkfun-pro-nrf52840-mini-hookup-guide/introduction)

[Matter over Thread with nRF52840](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/matter/index.html)

[OpenThread by Google](https://openthread.io)
