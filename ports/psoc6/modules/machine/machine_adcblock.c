#include <stdio.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#include "machine_adcblock.h"
#include "pins.h"
#include "machine_adc.h"

STATIC const ch_pin_map_t ch_pin_obj[] = {
    {.ch = 0, .pin = PIN_P10_0},
    {.ch = 1, .pin = PIN_P10_1},
    {.ch = 2, .pin = PIN_P10_2},
    {.ch = 3, .pin = PIN_P10_3},
    {.ch = 4, .pin = PIN_P10_4},
    {.ch = 5, .pin = PIN_P10_5},
};

STATIC void machine_adcblock_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADCBlock(%u, bits=%u)", self->adc_id, self->bits);
}

STATIC mp_obj_t machine_adcblock_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *all_args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Get ADC ID
    uint8_t adc_id = mp_obj_get_int(all_args[0]);
    if (adc_id != 0) {
        mp_raise_TypeError(MP_ERROR_TEXT("Specified ADC id not supported. Currently only block 0 is configured!"));
    }
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw_args, all_args + n_pos_args);

    // Get ADC bits
    enum { ARG_bits };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_bits, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_ADC_BITS} }
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_pos_args - 1, all_args + 1, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    uint8_t bits = args[ARG_bits].u_int;
    if (bits != DEFAULT_ADC_BITS) {
        mp_raise_TypeError(MP_ERROR_TEXT("Invalid bits. Current ADC configuration supports only 12 bits resolution!"));
    }

    // Specifics of adcblock object
    cyhal_adc_channel_t adc_chan_obj;

    machine_adcblock_obj_t *self = mp_obj_malloc(machine_adcblock_obj_t, &machine_adcblock_type);
    self->adc_id = adc_id;
    self->bits = bits;
    self->adc_chan_obj = adc_chan_obj;

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_adcblock_connect(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    uint8_t channel = -1;
    if (n_pos_args == 2) {
        // If channel only specified : If mp_obj_is_int is true, then it is channel
        if (mp_obj_is_int(pos_args[1])) {
            channel = mp_obj_get_int(pos_args[1]);
            if (channel <= 7) {
                // adc_pin
                // self->ch = mp_obj_get_int(pos_args[1]);
                self->adc_pin = ch_pin_obj[channel].pin;
            }
        }
        // If Pin only specified
        else {
            machine_pin_obj_t *adc_pin_obj = MP_OBJ_TO_PTR(pos_args[1]);

            for (int i = 0; i < MP_ARRAY_SIZE(ch_pin_obj); i++)
            {
                if (ch_pin_obj[i].pin == adc_pin_obj->pin_addr) {
                    // self->ch = ch_pin_obj[i].ch;
                    self->adc_pin = adc_pin_obj->pin_addr;
                }
            }
        }
    } else if (n_pos_args == 3) {
        self->ch = mp_obj_get_int(pos_args[1]);
        machine_pin_obj_t *adc_pin_obj = MP_OBJ_TO_PTR(pos_args[2]);
        self->adc_pin = adc_pin_obj->pin_addr;
        if (ch_pin_obj[self->ch].pin != self->adc_pin) {
            mp_raise_TypeError(MP_ERROR_TEXT("Wrong pin specified for the mentioned channel"));
        }
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Too many positional args"));
    }

    return adc_init_helper(1000, self->adc_pin, self->bits); // Default sampling time in ns = 1000
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_adcblock_connect_obj, 2, machine_adcblock_connect);


STATIC const mp_rom_map_elem_t machine_adcblock_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&machine_adcblock_connect_obj) },
};
STATIC MP_DEFINE_CONST_DICT(machine_adcblock_locals_dict, machine_adcblock_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_adcblock_type,
    MP_QSTR_ADCBlock,
    MP_TYPE_FLAG_NONE,
    make_new, machine_adcblock_make_new,
    print, machine_adcblock_print,
    locals_dict, &machine_adcblock_locals_dict
    );
