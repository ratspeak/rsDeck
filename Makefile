STANDALONE_ENV ?= rsdeck_915
RNODE_DIR ?= vendor/rnode_firmware
LAUNCHER_DIR ?= launcher
BUILD_DIR ?= build
DIST_DIR ?= dist

PARTITION_CSV := partitions/rsdeck_16mb_dual.csv
PARTITIONS_BIN := $(BUILD_DIR)/rsdeck_16mb_dual_partitions.bin

FULL_NAME := rsdeck-full
STANDALONE_NAME := rsdeck-standalone
RNODE_ONLY_NAME := rsdeck-rnode
M5LAUNCHER_STANDALONE_NAME := rsdeck-standalone-m5launcher
M5LAUNCHER_RNODE_NAME := rsdeck-rnode-m5launcher

FULL_BIN := $(BUILD_DIR)/$(FULL_NAME).bin
STANDALONE_BIN := $(BUILD_DIR)/$(STANDALONE_NAME).bin
RNODE_ONLY_BIN := $(BUILD_DIR)/$(RNODE_ONLY_NAME).bin

LAUNCHER_BIN := $(LAUNCHER_DIR)/.pio/build/tdeck_launcher/firmware.bin
STANDALONE_APP_BIN := .pio/build/$(STANDALONE_ENV)/firmware.bin
STANDALONE_FACTORY_BIN := rsdeck-merged.bin

RNODE_OUTPUT := $(RNODE_DIR)/build/tdeck.esp32.esp32s3
RNODE_BIN := $(RNODE_OUTPUT)/RNode_Firmware.ino.bin
RNODE_BOOTLOADER_BIN := $(RNODE_OUTPUT)/RNode_Firmware.ino.bootloader.bin
RNODE_PARTITIONS_BIN := $(RNODE_OUTPUT)/RNode_Firmware.ino.partitions.bin

BOOTLOADER_BIN := .pio/build/$(STANDALONE_ENV)/bootloader.bin
PLATFORMIO_ARDUINO ?= $(HOME)/.platformio/packages/framework-arduinoespressif32
ARDUINO15_ESP32 ?= $(HOME)/Library/Arduino15/packages/esp32/hardware/esp32/2.0.17
GEN_ESPPART ?= $(if $(wildcard $(PLATFORMIO_ARDUINO)/tools/gen_esp32part.py),$(PLATFORMIO_ARDUINO)/tools/gen_esp32part.py,$(ARDUINO15_ESP32)/tools/gen_esp32part.py)
BOOT_APP0_BIN ?= $(if $(wildcard $(PLATFORMIO_ARDUINO)/tools/partitions/boot_app0.bin),$(PLATFORMIO_ARDUINO)/tools/partitions/boot_app0.bin,$(ARDUINO15_ESP32)/tools/partitions/boot_app0.bin)

PORT ?= $(port)
ifeq ($(PORT),)
PORT := /dev/ttyACM0
endif

.PHONY: all build prep-tdeck build-standalone build-launcher build-rnode check bundle full-image standalone-image rnode-only-image package release flash clean

all: bundle

build: package

prep-tdeck:
	$(MAKE) -C $(RNODE_DIR) prep-esp32

build-standalone:
	python3 -m platformio run -e $(STANDALONE_ENV)

build-launcher:
	python3 -m platformio run -d $(LAUNCHER_DIR) -e tdeck_launcher

build-rnode:
	$(MAKE) -C $(RNODE_DIR) firmware-tdeck

$(PARTITIONS_BIN): $(PARTITION_CSV)
	mkdir -p $(BUILD_DIR)
	python3 $(GEN_ESPPART) $(PARTITION_CSV) $(PARTITIONS_BIN)

check: build-launcher build-standalone build-rnode
	python3 tools/check_image_fit.py --launcher $(LAUNCHER_BIN) --standalone $(STANDALONE_APP_BIN) --rnode $(RNODE_BIN)

full-image: check $(PARTITIONS_BIN)
	python3 tools/make_dual_image.py \
		--bootloader $(BOOTLOADER_BIN) \
		--partitions $(PARTITIONS_BIN) \
		--boot-app0 $(BOOT_APP0_BIN) \
		--launcher $(LAUNCHER_BIN) \
		--standalone $(STANDALONE_APP_BIN) \
		--rnode $(RNODE_BIN) \
		--output $(FULL_BIN)

standalone-image: build-standalone
	mkdir -p $(BUILD_DIR)
	cp $(STANDALONE_FACTORY_BIN) $(STANDALONE_BIN)

rnode-only-image: build-rnode
	mkdir -p $(BUILD_DIR)
	python3 -m esptool --chip esp32s3 merge-bin \
		--flash-mode dio --flash-size 16MB \
		--output $(RNODE_ONLY_BIN) \
		0x0000 $(RNODE_BOOTLOADER_BIN) \
		0x8000 $(RNODE_PARTITIONS_BIN) \
		0xe000 $(BOOT_APP0_BIN) \
		0x10000 $(RNODE_BIN)

bundle: full-image

package: full-image standalone-image rnode-only-image
	mkdir -p $(DIST_DIR)
	rm -f $(DIST_DIR)/$(FULL_NAME).zip \
	      $(DIST_DIR)/$(STANDALONE_NAME).zip \
	      $(DIST_DIR)/$(RNODE_ONLY_NAME).zip \
	      $(DIST_DIR)/$(M5LAUNCHER_STANDALONE_NAME).bin \
	      $(DIST_DIR)/$(M5LAUNCHER_RNODE_NAME).bin \
	      $(DIST_DIR)/rsdeck-standalone-app.bin \
	      $(DIST_DIR)/rsdeck-rnode-app.bin
	python3 tools/package_merged_zip.py --image $(FULL_BIN) --name $(FULL_NAME) --output $(DIST_DIR)/$(FULL_NAME).zip
	python3 tools/package_merged_zip.py --image $(STANDALONE_BIN) --name $(STANDALONE_NAME) --output $(DIST_DIR)/$(STANDALONE_NAME).zip
	python3 tools/package_merged_zip.py --image $(RNODE_ONLY_BIN) --name $(RNODE_ONLY_NAME) --output $(DIST_DIR)/$(RNODE_ONLY_NAME).zip
	cp $(STANDALONE_APP_BIN) $(DIST_DIR)/$(M5LAUNCHER_STANDALONE_NAME).bin
	cp $(RNODE_BIN) $(DIST_DIR)/$(M5LAUNCHER_RNODE_NAME).bin

release: package

flash: bundle
	python3 -m esptool --chip esp32s3 --port $(PORT) --baud 460800 --before default_reset --after hard_reset write-flash 0x0 $(FULL_BIN)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) $(STANDALONE_FACTORY_BIN)
