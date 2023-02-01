MCU_SERIES = MIMXRT1021
MCU_VARIANT = MIMXRT1021DAG5A

MICROPY_FLOAT_IMPL = double
MICROPY_PY_MACHINE_SDCARD = 1
MICROPY_HW_FLASH_TYPE = qspi_nor_flash
MICROPY_HW_FLASH_SIZE = 0x800000  # 8MB

MICROPY_HW_SDRAM_AVAIL = 1
MICROPY_HW_SDRAM_SIZE  = 0x2000000  # 32MB

MICROPY_PY_LWIP = 1
MICROPY_PY_USSL = 1
MICROPY_SSL_MBEDTLS = 1

FROZEN_MANIFEST ?= $(BOARD_DIR)/manifest.py

JLINK_PATH ?= /media/RT1020-EVK/
JLINK_COMMANDER_SCRIPT = $(BUILD)/script.jlink

ifdef JLINK_IP
JLINK_CONNECTION_SETTINGS = -IP $(JLINK_IP)
else
JLINK_CONNECTION_SETTINGS = -USB
endif

deploy_jlink: $(BUILD)/firmware.hex
	$(Q)$(TOUCH) $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "ExitOnError 1" > $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "speed auto" >> $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "r" >> $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "st" >> $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "loadfile \"$(realpath $(BUILD)/firmware.hex)\"" >> $(JLINK_COMMANDER_SCRIPT)
	$(ECHO) "qc" >> $(JLINK_COMMANDER_SCRIPT)
	$(JLINK_PATH)JLinkExe -device $(MCU_VARIANT) -if SWD $(JLINK_CONNECTION_SETTINGS) -CommanderScript $(JLINK_COMMANDER_SCRIPT)

deploy: $(BUILD)/firmware.bin
	cp $< $(JLINK_PATH)
