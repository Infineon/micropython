
# get path to this file
MPY_PATH_TO_MTB_ADAPTER_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
MPY_DIR_OF_MTB_ADAPTER_MAKEFILE  := $(dir $(MPY_PATH_TO_MTB_ADAPTER_MAKEFILE))


# get variable definitions from main makefile
MTB_BASE_EXAMPLE_MAKEFILE_DIR := $(MPY_DIR_OF_MTB_ADAPTER_MAKEFILE)
MPY_MTB_MAIN_MAKEFILE := $(MTB_BASE_EXAMPLE_MAKEFILE_DIR)/Makefile
MPY_MTB_TARGET        := $(shell egrep '^ *TARGET' $(MPY_MTB_MAIN_MAKEFILE) | sed 's/^.*= *//g')
MPY_MTB_CONFIG        ?= $(shell egrep '^ *CONFIG' $(MPY_MTB_MAIN_MAKEFILE) | sed 's/^.*= *//g')

MPY_MTB_BOARD_BUILD_DIR        := $(MTB_BASE_EXAMPLE_MAKEFILE_DIR)/$(BUILD)
MPY_MTB_BOARD_BUILD_OUTPUT_DIR := $(MPY_MTB_BOARD_BUILD_DIR)/$(MPY_MTB_TARGET)/$(MPY_MTB_CONFIG)

MPY_MTB_LIB_NAME       = $(file < $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR)/artifact.rsp)



# using WSL : should be something like /mnt/c/Users/???/ModusToolbox  with ??? to be replaced by user windows home directory
# MTB_WIN_HOME ?= $(OLDPWD)/ModusToolbox
# MTB_HOME     ?= $(HOME)/ModusToolbox

OPENOCD_HOME ?= $(HOME)/ModusToolbox/tools_3.0/openocd
# OPENOCD_WIN_HOME ?= $(MTB_WIN_HOME)/tools_3.0/openocd


$(info MPY_DIR_OF_MTB_ADAPTER_MAKEFILE  : $(MPY_DIR_OF_MTB_ADAPTER_MAKEFILE))

$(info MPY_MTB_MAIN_MAKEFILE            : $(MPY_MTB_MAIN_MAKEFILE))
$(info MPY_MTB_TARGET                   : $(MPY_MTB_TARGET))
$(info MPY_MTB_CONFIG                   : $(MPY_MTB_CONFIG))
$(info MTB_LIB_NAME                     : $(MPY_MTB_LIB_NAME))

$(info MPY_MTB_BOARD_BUILD_DIR          : $(MPY_MTB_BOARD_BUILD_DIR))
$(info MPY_MTB_BOARD_BUILD_OUTPUT_DIR   : $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR))

mtb_init: mtb_add_bsp mtb_set_bsp mtb_get_libs
	$(info )
	$(info Initializing ModusToolbox libs for board $(BOARD))

mtb_get_libs:
	$(info )
	$(info Retrieving ModusToolbox dependencies ...)
	$(Q) $(MAKE) -C $(MTB_BASE_EXAMPLE_MAKEFILE_DIR) getlibs

mtb_add_bsp:
	$(info )
	$(info Adding board $(BOARD) dependencies ...)
	$(Q) cd $(MTB_BASE_EXAMPLE_MAKEFILE_DIR); library-manager-cli --project . --add-bsp-name $(BOARD) --add-bsp-version $(BOARD_VERSION)

mtb_set_bsp: 
	$(info )
	$(info Setting board $(BOARD) as active ...)
	$(Q) cd $(MTB_BASE_EXAMPLE_MAKEFILE_DIR); library-manager-cli --project . --set-active-bsp APP_$(BOARD)

# Remove MTB retrieved lib and dependencies
mtb_deinit: mtb_clean
	$(info )
	$(info Removing mtb_shared and libs folder ...)
	-$(Q) cd $(MTB_BASE_EXAMPLE_MAKEFILE_DIR); rm -rf libs
	-$(Q) cd $(MTB_BASE_EXAMPLE_MAKEFILE_DIR); rm -rf bsps
	-$(Q) cd $(MTB_BASE_EXAMPLE_MAKEFILE_DIR); rm -rf ../mtb_shared

# build MTB project
mtb_build:
	$(info )
	$(info Building $(BOARD) in $(CONFIG) mode using MTB ...)
	$(Q) $(MAKE) -C $(MTB_BASE_EXAMPLE_MAKEFILE_DIR) CONFIG=$(MPY_MTB_CONFIG) build


mtb_clean:
	$(info )
	$(info Removing $(MPY_MTB_BOARD_BUILD_DIR) ...)
	-$(Q) rm -rf $(MPY_MTB_BOARD_BUILD_DIR)

# get variables set for includes, objects, etc. and add to mpy makefile variables
mtb_get_build_flags: mtb_build
	@:
	$(eval MPY_MTB_INCLUDE_DIRS = $(file < $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR)/inclist.rsp))
	$(eval INC += $(subst -I,-I$(MTB_BASE_EXAMPLE_MAKEFILE_DIR)/,$(MPY_MTB_INCLUDE_DIRS)))
	$(eval INC += -I$(BOARD_DIR))
	$(eval MPY_MTB_LIBRARIES = $(file < $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR)/liblist.rsp))
	$(eval LIBS += $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR)/$(MPY_MTB_LIB_NAME))
#	$(eval LIBS += $(MPY_MTB_LIBRARIES) $(MPY_MTB_BOARD_BUILD_OUTPUT_DIR)/$(MPY_MTB_LIB_NAME))
	$(eval CFLAGS +=  -mcpu=cortex-m4 --specs=nano.specs -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -ffunction-sections -fdata-sections -ffat-lto-objects -g -Wall -pipe -DCOMPONENT_4343W -DCOMPONENT_APP_CY8CPROTO_062_4343W -DCOMPONENT_CAT1 -DCOMPONENT_CAT1A -DCOMPONENT_CM0P_SLEEP -DCOMPONENT_CM4 -DCOMPONENT_CM4_0 -DCOMPONENT_Debug -DCOMPONENT_GCC_ARM -DCOMPONENT_HCI_UART -DCOMPONENT_MURATA_1DX -DCOMPONENT_MW_ABSTRACTION_RTOS -DCOMPONENT_MW_CAT1CM0P -DCOMPONENT_MW_CLIB_SUPPORT -DCOMPONENT_MW_CMSIS -DCOMPONENT_MW_CONNECTIVITY_UTILITIES -DCOMPONENT_MW_CORE_LIB -DCOMPONENT_MW_CORE_MAKE -DCOMPONENT_MW_CY_MBEDTLS_ACCELERATION -DCOMPONENT_MW_FREERTOS -DCOMPONENT_MW_LWIP -DCOMPONENT_MW_LWIP_FREERTOS_INTEGRATION -DCOMPONENT_MW_LWIP_NETWORK_INTERFACE_INTEGRATION -DCOMPONENT_MW_MBEDTLS -DCOMPONENT_MW_MTB_HAL_CAT1 -DCOMPONENT_MW_MTB_PDL_CAT1 -DCOMPONENT_MW_RECIPE_MAKE_CAT1A -DCOMPONENT_MW_RETARGET_IO -DCOMPONENT_MW_SECURE_SOCKETS -DCOMPONENT_MW_SERIAL_FLASH -DCOMPONENT_MW_WHD_BSP_INTEGRATION -DCOMPONENT_MW_WIFI_CONNECTION_MANAGER -DCOMPONENT_MW_WIFI_CORE_FREERTOS_LWIP_MBEDTLS -DCOMPONENT_MW_WIFI_HOST_DRIVER -DCOMPONENT_MW_WPA3_EXTERNAL_SUPPLICANT -DCOMPONENT_PSOC6_02 -DCOMPONENT_SOFTFP -DCOMPONENT_WIFI_INTERFACE_SDIO -DCORE_NAME_CM4_0=1 -DCY8C624ABZI_S2D44 -DCYBSP_WIFI_CAPABLE -DCY_APPNAME_mtb_example_wifi_scan -DCY_RETARGET_IO_CONVERT_LF_TO_CRLF -DCY_RTOS_AWARE -DCY_SUPPORTS_DEVICE_VALIDATION -DCY_TARGET_BOARD=APP_CY8CPROTO_062_4343W -DCY_USING_HAL -DCY_WIFI_HOST_WAKE_SW_FORCE=0 -DDEBUG -DMBEDTLS_USER_CONFIG_FILE=mbedtls_user_config.h -DTARGET_APP_CY8CPROTO_062_4343W  -DCOMPONENT_FREERTOS  -DCOMPONENT_LWIP -DCOMPONENT_MBEDTLS)
	$(eval CXXFLAGS += $(CFLAGS))
	$(eval LDFLAGS += -mcpu=cortex-m4 --specs=nano.specs -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -ffunction-sections -fdata-sections -ffat-lto-objects -g -Wall -pipe -Wl,--gc-sections -T$(MTB_BASE_EXAMPLE_MAKEFILE_DIR)/bsps/TARGET_APP_CY8CPROTO-062-4343W/COMPONENT_CM4/TOOLCHAIN_GCC_ARM/linker.ld -Wl,-Map,$(BUILD)/firmware.map -Wl,--start-group -Wl,--end-group -Wl,--print-memory-usage)
	$(eval QSTR_GEN_CFLAGS += $(INC) $(CFLAGS))

attached_devs:
	@:
	$(eval ATTACHED_TARGET_LIST = $(shell $(MTB_HOME)/tools_3.0/fw-loader/bin/fw-loader --device-list | sed -n -e 's/[0-9]: KitProg3 CMSIS-DAP BULK-//' -e 's/FW Version[^0-9]*\(\([0-9]\.\)\{0,4\}[0-9][^.]\).*//p'| sed -n 's/^[ \t]*//p'))
	$(eval ATTACHED_TARGETS_NUMBER = $(words $(ATTACHED_TARGET_LIST)))
	$(info Number of attached targets : $(ATTACHED_TARGETS_NUMBER))
	$(info List of attached targets : $(ATTACHED_TARGET_LIST))

program: $(MPY_MAIN_BUILD_DIR)/firmware.hex
	@:
	$(info )
	$(info Programming using openocd ...)
	openocd -s $(OPENOCD_HOME)/scripts -s $(MTB_BASE_EXAMPLE_MAKEFILE_DIR)/bsps/TARGET_APP_$(BOARD)/config/GeneratedSource -c "source [find interface/kitprog3.cfg]; $(SERIAL_ADAPTER_CMD) ; source [find target/psoc6_2m.cfg]; psoc6 allow_efuse_program off; psoc6 sflash_restrictions 1; program $(MPY_DIR_OF_MAIN_MAKEFILE)/build/firmware.hex verify reset exit;"
	$(info Programming done.)

# Use this target to program multiple attached target devices
program_multi: attached_devs
	@:
	$(foreach ATTACHED_TARGET, $(ATTACHED_TARGET_LIST), $(MAKE) mpy_program SERIAL_ADAPTER_CMD='adapter serial $(ATTACHED_TARGET)';)

program_ext_hex:
	@:
	$(info )
	$(info Programming using openocd ...)
	openocd -s $(OPENOCD_HOME)/scripts -s $(MPY_DIR_OF_MTB_ADAPTER_MAKEFILE)/bsps/TARGET_APP_$(BOARD)/config/GeneratedSource -c "source [find interface/kitprog3.cfg]; $(SERIAL_ADAPTER_CMD) ; source [find target/psoc6_2m.cfg]; psoc6 allow_efuse_program off; psoc6 sflash_restrictions 1; program $(EXT_HEX_FILE) verify reset exit;"
	$(info Programming done.)

program_multi_ext_hex: attached_devs
	@:
	$(foreach ATTACHED_TARGET, $(ATTACHED_TARGET_LIST), $(MAKE) mpy_program_ext_hex SERIAL_ADAPTER_CMD='adapter serial $(ATTACHED_TARGET)' $(EXT_HEX_FILE);)


.PHONY: mtb_init mtb_deinit mtb_build mtb_get_build_flags program program_multi program_ext_hex program_multi_ext_hex