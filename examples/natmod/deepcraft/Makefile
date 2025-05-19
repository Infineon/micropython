# Location of top-level MicroPython directory
MPY_DIR = ../../..

# Name of module
MOD = deepcraft_model

# Source files (.c or .py)
SRC = dc_mp_iface.c model.c

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin, rv32imc)
ARCH = armv7emsp

# Link with libm.a and libgcc.a from the toolchain
LINK_RUNTIME = 1

override LIBGCC_PATH := gcc/lib/gcc/arm-none-eabi/11.3.1/thumb/v7e-m+fp/hard/libgcc.a
override LIBM_PATH := gcc/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libm.a

include $(MPY_DIR)/py/dynruntime.mk

# Custom clean target
clean:
	rm -rf .mpy_ld_cache/ build/ $(MOD).mpy