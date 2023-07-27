#include <stdio.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#include "machine_adcblock.h"
#include "machine_adc.h"
#include "pins.h"

extern machine_adc_obj_t *adc_init_helper(uint32_t sampling_time, uint32_t pin);

machine_adcblock_obj_t *adc_block[MAX_BLOCKS] = {NULL};

const adc_block_channel_pin_map_t adc_block_pin_map[] = {
    {ADCBLOCK0, 0, PIN_P10_0},
    {ADCBLOCK0, 1, PIN_P10_1},
    {ADCBLOCK0, 2, PIN_P10_2},
    {ADCBLOCK0, 3, PIN_P10_3},
    {ADCBLOCK0, 4, PIN_P10_4},
    {ADCBLOCK0, 5, PIN_P10_5}
}; // will belong to only a particular bsp

static inline bool is_valid_id(uint8_t adc_id) {
    if (adc_id >= MAX_BLOCKS) {
        return false;
    }

    return true;
}

static inline machine_adcblock_obj_t *adc_block_get_allocated(uint8_t adc_block_id) {
    for (uint8_t i = 0; i < MAX_BLOCKS; i++)
    {
        if (adc_block[i] != NULL) {
            if (adc_block[i]->id == adc_block_id) {
                return adc_block[i];
            }
        }
    }

    return NULL;
}

static inline uint8_t adc_block_get_free_block() {
    uint8_t i;
    for (i = 0; i < MAX_BLOCKS; i++)
    {
        if (adc_block[i] == NULL) {
            break;
        }
    }

    return i;
}

static inline uint32_t adc_block_get_any_assoc_pin(uint16_t adc_block_id) {
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        if (adc_block_pin_map[i].block_id == adc_block_id) {
            return adc_block_pin_map[i].pin;
        }
    }

    return 0; // there should always an associated pin for block...
}

static inline machine_adcblock_obj_t *adc_block_init_new(uint16_t adc_block_id, uint8_t bits) {
    uint8_t free_index = adc_block_get_free_block();
    // TODO: Associate id to index to simplify. Block always exists then, only is it inited or not.
    if (free_index <= MAX_BLOCKS) {
        adc_block[free_index] = mp_obj_malloc(machine_adcblock_obj_t, &machine_adcblock_type);
        adc_block[free_index]->id = adc_block_id;
        adc_block[free_index]->bits = bits;

        cyhal_adc_init(&(adc_block[free_index]->adc_block_obj), adc_block_get_any_assoc_pin(adc_block_id), NULL);
        // TODO: handle return of cyhal

        printf("\n Block created! \n");

        return adc_block[free_index];
    }

    return NULL;
}

// Get adc block id associated to input pin
int16_t get_adc_block_id(uint32_t pin) {
    // printf("\n Size of adc_block_pin_map: %d", sizeof(adc_block_pin_map)/sizeof(adc_block_pin_map[0]));
    for (int i = 0; i < (sizeof(adc_block_pin_map) / sizeof(adc_block_pin_map[0])); i++)
    {
        if (pin == adc_block_pin_map[i].pin) {
            return adc_block_pin_map[i].block_id;
        }
    }
    return -1;
}

// Helper function to get channel number provided the pin is given
int16_t get_adc_channel_number(uint32_t pin) {
    for (int i = 0; i < sizeof(adc_block_pin_map); i++)
    {
        if (pin == adc_block_pin_map[i].pin) {
            return adc_block_pin_map[i].channel;
        }
    }
    return -1;
}

machine_adc_obj_t *adc_block_get_channel_obj(machine_adcblock_obj_t *adc_block, uint8_t channel) {
    return adc_block->channel[channel];
}

machine_adc_obj_t *adc_block_allocate_new_pin(machine_adcblock_obj_t *adc_block, uint32_t pin) {
    int16_t adc_channel_no = get_adc_channel_number(pin);
    adc_block->channel[adc_channel_no] = mp_obj_malloc(machine_adc_obj_t, &machine_adc_type);

    return adc_block->channel[adc_channel_no];
}

machine_adc_obj_t *adc_block_get_pin_adc_obj(machine_adcblock_obj_t *adc_block, uint32_t pin) {

    int16_t adc_channel_no = get_adc_channel_number(pin);

    return adc_block->channel[adc_channel_no];
}

machine_adcblock_obj_t *adc_block_init_helper(uint8_t adc_id, uint8_t bits) {

    if (!is_valid_id(adc_id)) {
        mp_raise_TypeError(MP_ERROR_TEXT("Specified ADC id not supported"));
    }

    machine_adcblock_obj_t *adc_block = adc_block_get_allocated(adc_id);

    if (adc_block == NULL) {
        adc_block = adc_block_init_new(adc_id, bits);
    }

    if (adc_block == NULL) {
        mp_raise_TypeError(MP_ERROR_TEXT("ADC Blocks not available. Fully allocated"));
    }

    return adc_block;
}

STATIC void machine_adcblock_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADCBlock(%u, bits=%u)", self->id, self->bits);
}

STATIC mp_obj_t machine_adcblock_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *all_args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Get ADC ID
    uint8_t adc_id = mp_obj_get_int(all_args[0]);

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

    return MP_OBJ_FROM_PTR(adc_block_init_helper(adc_id, bits));
}

/*STATIC mp_obj_t machine_adcblock_connect(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_adcblock_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    uint8_t channel = -1;
    if (n_pos_args == 2) {
        // TODO: generalize for (block, channel, pin) structure
        // If channel only specified : If mp_obj_is_int is true, then it is channel
        if (mp_obj_is_int(pos_args[1])) {
            channel = mp_obj_get_int(pos_args[1]);
            if (channel <= 7) {
                self->adc_pin = adc_block_pin_map[channel].pin;
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
*/

STATIC const mp_rom_map_elem_t machine_adcblock_locals_dict_table[] = {
    // { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&machine_adcblock_connect_obj) },
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
