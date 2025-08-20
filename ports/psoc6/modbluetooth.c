// All includes may or may not be needed
#include "py/binary.h"
#include "py/gc.h"
#include "py/misc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "py/qstr.h"
#include "py/runtime.h"


#include "cycfg_bt_settings.h"
#include "modbluetooth.h"

#include "wiced_bt_stack.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>
#include "cybt_platform_trace.h"
#include "wiced_memory.h"
#include "cyhal.h"
#include "stdio.h"
#include "cycfg_gatt_db.h"
#include "cycfg_gap.h"
#include "wiced_bt_dev.h"
#include "cybsp_bt_config.h"



// ******************************************************************************
typedef struct {
    mp_obj_base_t base;
} mp_obj_bluetooth_ble_t;

static const mp_obj_type_t mp_type_bluetooth_ble;

static mp_obj_t bluetooth_ble_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_printf(&mp_plat_print, "BLE constructor invoked\n");
    // !ToDo: Needs complete implementation / refactor when corresponding features are implemented
    mp_obj_bluetooth_ble_t *self = m_new0(mp_obj_bluetooth_ble_t, 1);
    self->base.type = &mp_type_bluetooth_ble;
    MP_STATE_VM(bluetooth) = MP_OBJ_FROM_PTR(self);
    return MP_STATE_VM(bluetooth);
}

wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data) {
    /*wiced_result_t wiced_result = WICED_BT_SUCCESS;
    wiced_bt_device_address_t bda = { 0 };

    switch (event)
    {
        case BTM_ENABLED_EVT:
            if (WICED_BT_SUCCESS == p_event_data->enabled.status)
            {
                wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_PUBLIC);
                wiced_bt_dev_read_local_addr(bda);
                mp_printf(&mp_plat_print, "Local Bluetooth Address: ");
                print_bd_address(bda);

                le_app_init();
            }
            else
            {
                printf( "Bluetooth Disabled \n" );
            }

            break;

    }*/
    return WICED_BT_SUCCESS;

}



extern const cybt_platform_config_t cybsp_bt_platform_cfg;
int mp_bluetooth_init(void) {
    // Initialize the Bluetooth hardware and stack.
    mp_printf(&mp_plat_print, "mp_bluetooth_init invoked \n");
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* Register call back and configuration with stack */
    wiced_result_t wiced_result = wiced_bt_stack_init(app_bt_management_callback, NULL);  // &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */

    if (WICED_BT_SUCCESS == wiced_result) {
        mp_printf(&mp_plat_print, "mp_bluetooth_init done \n");
    } else {
        CY_ASSERT(0);
    }

    return 0; // Return 0 on success.

}

static mp_obj_t bluetooth_handle_errno(int err) {
    if (err != 0) {
        mp_raise_OSError(err);
    }
    return mp_const_none;
}

// BLE.active
static mp_obj_t bluetooth_ble_active(size_t n_args, const mp_obj_t *args) {
    mp_printf(&mp_plat_print, "BLE.active invoked\n");
    if (n_args == 2) {
        // Boolean enable/disable argument supplied, set current state.
        int err;
        if (mp_obj_is_true(args[1])) {
            err = mp_bluetooth_init();
        } else {
            // err = mp_bluetooth_deinit();
        }
        bluetooth_handle_errno(err);
    }
    return MP_OBJ_FROM_PTR(true);
    // Return current state.
    // return mp_obj_new_bool(mp_bluetooth_is_active());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(bluetooth_ble_active_obj, 1, 2, bluetooth_ble_active);


// ----------------------------------------------------------------------------
// Bluetooth object: Definition
// ----------------------------------------------------------------------------

static const mp_rom_map_elem_t bluetooth_ble_locals_dict_table[] = {
    // General
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&bluetooth_ble_active_obj) }
};
static MP_DEFINE_CONST_DICT(bluetooth_ble_locals_dict, bluetooth_ble_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_bluetooth_ble,
    MP_QSTR_BLE,
    MP_TYPE_FLAG_NONE,
    make_new, bluetooth_ble_make_new,
    locals_dict, &bluetooth_ble_locals_dict
    );


static const mp_rom_map_elem_t mp_module_bluetooth_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_bluetooth) },
    { MP_ROM_QSTR(MP_QSTR_BLE), MP_ROM_PTR(&mp_type_bluetooth_ble) },
};

static MP_DEFINE_CONST_DICT(mp_module_bluetooth_globals, mp_module_bluetooth_globals_table);

const mp_obj_module_t mp_module_bluetooth = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_bluetooth_globals,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_bluetooth, mp_module_bluetooth);

MP_REGISTER_ROOT_POINTER(mp_obj_t bluetooth);

// #endif
