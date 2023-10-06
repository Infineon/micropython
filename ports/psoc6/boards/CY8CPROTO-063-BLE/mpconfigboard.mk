FROZEN_MANIFEST ?= boards/manifest.py
MICROPY_PY_SSL = 0
MICROPY_PSOC6_SSL_MBEDTLS = 0
BOARD_VERSION=release-v4.2.0

# Variables to support make-pins
PIN_PACKAGE_FILE = cyhal_psoc6_01_116_bga_ble.h
GPIO_MTB_HEADER_FILE = gpio_psoc6_02_124_bga.h #ToDo: Get the right file name