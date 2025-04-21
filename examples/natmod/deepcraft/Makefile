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

CFLAGS += -Dsqrtf=__builtin_sqrtf -fno-math-errno -fno-trapping-math -fno-builtin-logf -ffreestanding
CFLAGS += --data-sections
# If -lm is required:
# Even stronger — exclude specific problematic objects
LDFLAGS += -Wl,--exclude-libs,ALL





#CFLAGS += -Dlogf=__builtin_logf -ffreestanding -fno-math-errno -fno-trapping-math -ffast-math

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk

# Custom clean target
clean:
	rm -rf .mpy_ld_cache/ build/ $(MOD).mpy