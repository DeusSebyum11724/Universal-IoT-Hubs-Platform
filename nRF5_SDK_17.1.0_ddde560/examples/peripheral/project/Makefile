SDK_ROOT := /Users/asbri/Downloads/DeviceDownload/nRF5_SDK_17.1.0_ddde560

TARGET := nrf52840_usb_cdc
OUTPUT_DIRECTORY := _build

GNU_INSTALL_ROOT := /Users/asbri/Downloads/gcc-arm-none-eabi-10.3-2021.10/bin/
GNU_PREFIX := arm-none-eabi

DEVICE := NRF52840_XXAA
BOARD := PCA10056

C_SOURCE_FILES += main.c

CFLAGS += -DBOARD_$(BOARD)
CFLAGS += -mcpu=cortex-m4 -mthumb -mabi=aapcs -Wall -O2 -g3 -std=c99

CFLAGS += -I$(SDK_ROOT)/components
CFLAGS += -I$(SDK_ROOT)/components/boards
CFLAGS += -I$(SDK_ROOT)/components/libraries/usbd
CFLAGS += -I$(SDK_ROOT)/components/libraries/usbd/class/cdc
CFLAGS += -I$(SDK_ROOT)/components/libraries/fifo
CFLAGS += -I$(SDK_ROOT)/components/libraries/timer
CFLAGS += -I$(SDK_ROOT)/components/libraries/log
CFLAGS += -I$(SDK_ROOT)/components/drivers_nrf/usbd
CFLAGS += -I$(SDK_ROOT)/integration/nrfx/legacy
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/drivers/include
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/
CFLAGS += -I$(SDK_ROOT)/components/libraries/delay
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/templates/nRF52820
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/mdk
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/templates
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/hal
CFLAGS += -I$(SDK_ROOT)/components/libraries/util
CFLAGS += -I$(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd
CFLAGS += -I$(SDK_ROOT)/config/nrf52840/config
CFLAGS += -I$(SDK_ROOT)/components/libraries/usbd/class/cdc/acm

SDK_CONFIG_FILE := sdk_config.h

all:
	@mkdir -p $(OUTPUT_DIRECTORY)
	@echo "Building project..."
	$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-gcc $(CFLAGS) \
	    -o $(OUTPUT_DIRECTORY)/$(TARGET).out \
	    $(C_SOURCE_FILES) \
	    -include $(SDK_CONFIG_FILE) \
	    -T$(SDK_ROOT)/components/toolchain/gcc/gcc_nrf52.ld \
	    -L$(GNU_INSTALL_ROOT)/../lib/gcc/arm-none-eabi/10.3.1 \
	    -lc -lnosys -lm

clean:
	rm -rf $(OUTPUT_DIRECTORY)

BOOTLOADER_HEX := $(OUTPUT_DIRECTORY)/$(TARGET)_dfu.hex
BOOTLOADER_ZIP := $(OUTPUT_DIRECTORY)/$(TARGET)_dfu.zip
SERIAL_PORT ?= /dev/cu.usbmodem000000

dfu: $(BOOTLOADER_ZIP)

$(BOOTLOADER_HEX): $(OUTPUT_DIRECTORY)/$(TARGET).out
	@echo "Generating HEX..."
	arm-none-eabi-objcopy -O ihex $< $@

$(BOOTLOADER_ZIP): $(BOOTLOADER_HEX)
	nrfutil pkg generate --hw-version 52 --sd-req 0x00 --application $< --application-version 1 --key-file private.key $@

bootload: $(BOOTLOADER_ZIP)
	nrfutil dfu usb-serial -pkg $< -p $(SERIAL_PORT)

