#include <stdio.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#include "machine_adcblock.h"
#include "pins.h"
#include "machine_adc.h"


#define ADCBLOCK0               (0) 
#define ADCBLOCK_CHANNEL_MAX    (1)

STATIC machine_adblock_obj_t * acd_block[MAX_BLOCKS] = {NULL};

typedef struct 
{
    uint16_t block_id;
    uint16_t channel;
    uint16_t pin;
}acd_block_channel_pin_map_t;

STATIC const acd_block_channel_pin_map_t adc_block_pin_map[] = {
    {ADCBLOCK0, 0, PIN_P10_0},
    {ADCBLOCK0, 1, PIN_P10_1},
    {ADCBLOCK0, 2, PIN_P10_2},
    {ADCBLOCK0, 3, PIN_P10_3},
    {ADCBLOCK0, 4, PIN_P10_4},
    {ADCBLOCK0, 5, PIN_P10_5}
}; //will belong to only a particular bsp

STATIC void machine_adcblock_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADCBlock(%u, bits=%u)", self->adc_id, self->bits);
}

STATIC mp_obj_t machine_adcblock_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *all_args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Get ADC ID
    uint8_t adc_id = mp_obj_get_int(all_args[0]);
    // TODO: check if this id is a valid/avaiable genertically
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

    //TODO: check if the object already exists (the instance object)
    // if the corresponding index in the array isn´t NULL

    //TODO: If not construct the object
    machine_adcblock_obj_t *self = mp_obj_malloc(machine_adcblock_obj_t, &machine_adcblock_type);
    self->id = adc_id;
    self->bits = bits;
    // TODO: initialize chan array to NULL
    // self->adc_chan_obj =  {NULL};

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_adcblock_connect(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    uint8_t channel = -1;
    if (n_pos_args == 2) {
        // TODO: generalize for (block, channel, pin) structure
        // If channel only specified : If mp_obj_is_int is true, then it is channel
        if (mp_obj_is_int(pos_args[1])) {
            channel = mp_obj_get_int(pos_args[1]);
            if (channel <= 7) {
                self->adc_pin = ch_pin_obj[channel].pin;
            }
        }
        // TODO: generalize for (block, channel, pin) structure
        // If Pin only specified
        else {
            machine_pin_obj_t *adc_pin_obj = MP_OBJ_TO_PTR(pos_args[1]);

            for (int i = 0; i < MP_ARRAY_SIZE(ch_pin_obj); i++)
            {
                if (ch_pin_obj[i].pin == adc_pin_obj->pin_addr) {
                    self->adc_pin = adc_pin_obj->pin_addr;
                }
            }
        }
    } else if (n_pos_args == 3) {
        self->ch = mp_obj_get_int(pos_args[1]);
        machine_pin_obj_t *adc_pin_obj = MP_OBJ_TO_PTR(pos_args[2]);
        // TODO: generalize for (block, channel, pin) structure
        self->adc_pin = adc_pin_obj->pin_addr;
        if (ch_pin_obj[self->ch].pin != self->adc_pin) {
            mp_raise_TypeError(MP_ERROR_TEXT("Wrong pin specified for the mentioned channel"));
        }
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Too many positional args"));
    }

    //TODO: check if the adc_obj already exists

    //TODO: create the adc object
    return adc_init_helper(1000, self->adc_pin, self->bits); // Default sampling time in ns = 1000

    //TODO: allocate it in the right channel index of the array.
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
