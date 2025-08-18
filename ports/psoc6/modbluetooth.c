#include "py/binary.h"
#include "py/gc.h"
#include "py/misc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "py/qstr.h"
#include "py/runtime.h"


#include "ble/cycfg_bt_settings.h"
#include "modbluetooth.h"

#if MICROPY_PSOC6_BLUETOOTH

// ----------------------------------------------------------------------------
// Bluetooth object: General
// ----------------------------------------------------------------------------

static const mp_obj_type_t mp_type_bluetooth_ble;

static mp_obj_t bluetooth_ble_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_printf(&mp_plat_print, "To be implemented\n");
    return mp_const_none;
}

// ----------------------------------------------------------------------------
// UUID Function Implementation
// ----------------------------------------------------------------------------

/**
 * Function to create a UUID object.
 * This function is part of the BLE object and expects self as the first argument.
 */
typedef struct {
    mp_obj_base_t base;
    uint8_t type;
    uint8_t data[16];
} mp_obj_bluetooth_uuid_t;

extern const mp_obj_type_t mp_type_bluetooth_uuid;
// Constructor for the UUID class
static mp_obj_t bluetooth_uuid_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // Ensure exactly one argument (UUID value)
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_obj_t arg = all_args[0];

    // Create a new UUID object
    mp_obj_bluetooth_uuid_t *self = mp_obj_malloc(mp_obj_bluetooth_uuid_t, type);

    if (mp_obj_is_int(arg)) {
        // Handle 16-bit UUID
        uint16_t uuid16 = mp_obj_get_int(arg);
        if (uuid16 > 0xFFFF) {
            mp_raise_ValueError(MP_ERROR_TEXT("UUID must be a valid 16-bit value"));
        }
        self->type = 16;  // 16-bit UUID type
        self->data[0] = uuid16 & 0xFF;         // Low byte
        self->data[1] = (uuid16 >> 8) & 0xFF; // High byte
        mp_printf(&mp_plat_print, "Created 16-bit UUID: 0x%04x\n", uuid16);
    } else if (mp_obj_is_type(arg, &mp_type_bytes)) {
        // Handle 128-bit UUID
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(arg, &bufinfo, MP_BUFFER_READ);
        if (bufinfo.len != 16) {
            mp_raise_ValueError(MP_ERROR_TEXT("UUID must be 16 bytes long"));
        }
        self->type = 128;  // 128-bit UUID type
        memcpy(self->data, bufinfo.buf, 16);
        mp_printf(&mp_plat_print, "Created 128-bit UUID");
    } else {
        // Invalid type
        mp_raise_TypeError(MP_ERROR_TEXT("UUID must be an int (16-bit) or bytes (128-bit)"));
    }

    return MP_OBJ_FROM_PTR(self);
}

// UUID __str__ method to provide string representation
static void bluetooth_uuid_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_obj_bluetooth_uuid_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->type == 16) {
        // 16-bit UUID
        uint16_t uuid16 = (self->data[1] << 8) | self->data[0];
        mp_printf(print, "UUID(0x%04x)", uuid16);
    } else if (self->type == 128) {
        // 128-bit UUID
        mp_printf(print, "UUID(");
        for (int i = 0; i < 16; i++) {
            if (i > 0) {
                mp_printf(print, ":");
            }
            mp_printf(print, "%02x", self->data[i]);
        }
        mp_printf(print, ")");
    }
}
MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_bluetooth_uuid,
    MP_QSTR_UUID,
    MP_TYPE_FLAG_NONE,
    make_new, bluetooth_uuid_make_new,
    print, bluetooth_uuid_print
);
// ----------------------------------------------------------------------------
// Bluetooth object: Definition
// ----------------------------------------------------------------------------

static const mp_rom_map_elem_t bluetooth_ble_locals_dict_table[] = {
    // General
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
    { MP_ROM_QSTR(MP_QSTR_UUID), MP_ROM_PTR(&mp_type_bluetooth_uuid) },
};

static MP_DEFINE_CONST_DICT(mp_module_bluetooth_globals, mp_module_bluetooth_globals_table);

const mp_obj_module_t mp_module_bluetooth = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_bluetooth_globals,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_bluetooth, mp_module_bluetooth);

MP_REGISTER_ROOT_POINTER(mp_obj_t bluetooth);

#endif
