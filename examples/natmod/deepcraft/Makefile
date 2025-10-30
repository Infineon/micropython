# Location of top-level MicroPython directory
MPY_DIR = ../../..

# Name of module
MOD = deepcraft_model

# Source files (.c or .py)
SRC = dc_mp_iface.c

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin, rv32imc)
ARCH = armv7emsp

# DEEPCRAFT model type: starter (source) or ready (static library)
dcmodel ?= starter

# Link with libm.a and libgcc.a from the toolchain
LINK_RUNTIME = 1

# Conditional compilation based on dcmodel parameter

#ifeq ($(dcmodel), ready)
#    # Use static library for ready models
#    MPY_LD_FLAGS += -l./snore.a
#    CFLAGS += -DDCMODEthoL_READY=1
#    $(info Building with ready model using static library)
#else ifeq ($(dcmodel), starter)
#    # Use source file for starter models
#    SRC += model.c
#    CFLAGS += -DDCMODEL_STARTER=1
#    $(info Building with starter model using source compilation)
#else
#    $(error Invalid dcmodel value: $(dcmodel). Use 'starter' or 'ready')
#endif


#ifeq ($(dcmodel), ready)
#    # Use static library for ready models
#    MPY_LD_FLAGS += -l./snore.a
#    $(info Building with ready model using static library)
#else ifeq ($(dcmodel), starter)
#    # Use source file for starter models
#    SRC += model.c
#    $(info Building with starter model using source compilation)
#else
#    $(error Invalid dcmodel value: $(dcmodel). Use 'starter' or 'ready')
#endif

# OS-specific settings
ifeq ($(OS), Windows_NT)
    RM = del /Q /F
    CP = copy
    override MKDIR = cmd /C "if not exist $(1) mkdir $(1)"
    override PYTHON = python
    override ARCH_UPPER = $(shell powershell -Command "Write-Output '$(ARCH)'.ToUpper()")
    override MICROPY_FLOAT_IMPL_UPPER = $(shell powershell -Command "Write-Output '$(MICROPY_FLOAT_IMPL)'.ToUpper()")
    CLEAN_CMD = del /Q /F .mpy_ld_cache\* build\* $(MOD).mpy 2>nul
endif

CFLAGS += -Wno-error=implicit-function-declaration

override LIBGCC_PATH := gcc/lib/gcc/arm-none-eabi/11.3.1/thumb/v7e-m+fp/hard/libgcc.a
override LIBM_PATH := gcc/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libm.a

#MPY_LD_FLAGS += -l./dc_test.a
MPY_LD_FLAGS += -l./deepcraft_readymodel_snore_eval.a
#MPY_LD_FLAGS += -l./snore.a

include $(MPY_DIR)/py/dynruntime.mk

#$(BUILD_DIRS):
#	$(Q)$(call MKDIR, $@)
