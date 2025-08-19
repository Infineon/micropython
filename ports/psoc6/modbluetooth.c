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
    (void)type;

    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // Allocate memory for the UUID object
    mp_obj_bluetooth_uuid_t *self = mp_obj_malloc(mp_obj_bluetooth_uuid_t, &mp_type_bluetooth_uuid);

    if (mp_obj_is_int(all_args[0])) {
        // Handle 16-bit UUID
        self->type = MP_BLUETOOTH_UUID_TYPE_16;
        mp_int_t value = mp_obj_get_int(all_args[0]);
        if (value > 65535) {
            mp_raise_ValueError(MP_ERROR_TEXT("invalid UUID"));
        }
        self->data[0] = value & 0xff;
        self->data[1] = (value >> 8) & 0xff;

        // Debug: Print the 16-bit UUID
        mp_printf(&mp_plat_print, "Created 16-bit UUID: 0x%04x\n", value);

    } else {
        mp_buffer_info_t uuid_bufinfo = {0};
        mp_get_buffer_raise(all_args[0], &uuid_bufinfo, MP_BUFFER_READ);

        if (uuid_bufinfo.len == 2 || uuid_bufinfo.len == 4 || uuid_bufinfo.len == 16) {
            // Handle byte data (16-bit, 32-bit, or 128-bit UUID)
            self->type = uuid_bufinfo.len;
            memcpy(self->data, uuid_bufinfo.buf, self->type);

            if (uuid_bufinfo.len == 2) {
                // Debug: Print the 16-bit UUID
                uint16_t uuid16 = (self->data[1] << 8) | self->data[0];
                mp_printf(&mp_plat_print, "Created 16-bit UUID from bytes: 0x%04x\n", uuid16);
            } else if (uuid_bufinfo.len == 4) {
                // Debug: Print the 32-bit UUID
                uint32_t uuid32 = (self->data[3] << 24) | (self->data[2] << 16) | (self->data[1] << 8) | self->data[0];
                mp_printf(&mp_plat_print, "Created 32-bit UUID from bytes: 0x%08lx\n", uuid32);
            } else if (uuid_bufinfo.len == 16) {
                // Debug: Print the 128-bit UUID
                mp_printf(&mp_plat_print, "Created 128-bit UUID from bytes: ");
                for (size_t i = 0; i < 16; i++) {
                    mp_printf(&mp_plat_print, "%02x", self->data[i]);
                    if (i < 15) {
                        mp_printf(&mp_plat_print, ":");
                    }
                }
                mp_printf(&mp_plat_print, "\n");
            }

        } else {
            // Handle UUID string
            self->type = MP_BLUETOOTH_UUID_TYPE_128;
            int uuid_i = 32;
            for (size_t i = 0; i < uuid_bufinfo.len; i++) {
                char c = ((char *)uuid_bufinfo.buf)[i];
                if (c == '-') {
                    continue;
                }
                if (!unichar_isxdigit(c)) {
                    mp_raise_ValueError(MP_ERROR_TEXT("invalid char in UUID"));
                }
                c = unichar_xdigit_value(c);
                uuid_i--;
                if (uuid_i < 0) {
                    mp_raise_ValueError(MP_ERROR_TEXT("UUID too long"));
                }
                if (uuid_i % 2 == 0) {
                    // lower nibble
                    self->data[uuid_i / 2] |= c;
                } else {
                    // upper nibble
                    self->data[uuid_i / 2] = c << 4;
                }
            }
            if (uuid_i > 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("UUID too short"));
            }

            // Debug: Print the parsed 128-bit UUID from string
            mp_printf(&mp_plat_print, "Created 128-bit UUID from string: ");
            for (size_t i = 0; i < 16; i++) {
                mp_printf(&mp_plat_print, "%02x", self->data[i]);
                if (i < 15) {
                    mp_printf(&mp_plat_print, ":");
                }
            }
            mp_printf(&mp_plat_print, "\n");
        }
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
