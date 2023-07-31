
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"

// port-specific includes
#include "modmachine.h"
#include "pins.h"
#include "machine_adc.h"
#include "machine_adcblock.h"

#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"

#define DEFAULT_ADC_ACQ_NS  1000

#define IS_GPIO_VALID_ADC_PIN(gpio) ((gpio == CYHAL_NC_PIN_VALUE) || ((gpio >= 80) && (gpio <= 87)))

extern int16_t get_adc_block_id(uint32_t pin);
extern machine_adc_obj_t *adc_block_get_pin_adc_obj(machine_adcblock_obj_t *adc_block, uint32_t pin);
extern machine_adcblock_obj_t *adc_block_init_helper(uint8_t adc_id, uint8_t bits);
extern machine_adc_obj_t *adc_block_allocate_new_pin(machine_adcblock_obj_t *adc_block, uint32_t pin);

/******************************************************************************/
// MicroPython bindings for machine.ADC

const mp_obj_type_t machine_adc_type;

machine_adc_obj_t *adc_init_new(machine_adcblock_obj_t *adc_block, uint32_t pin, uint32_t sampling_time) {

    machine_adc_obj_t *o = adc_block_allocate_new_pin(adc_block, pin);
    const cyhal_adc_channel_config_t channel_config =
    {
        .enable_averaging = false,
        .min_acquisition_ns = sampling_time, // TODO: if existing, can we change its configuration (sampling rate?)
        .enabled = true
    };

    cyhal_adc_channel_init_diff(&(o->adc_chan_obj), &(adc_block->adc_block_obj), pin, CYHAL_ADC_VNEG, &channel_config);
    // TODO: handle error return

    o->pin = pin;
    o->block = adc_block;
    o->sample_ns = sampling_time;

    return o;
}

// machine_adc_print()
STATIC void machine_adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<ADC Pin=%u, ADCBlock_id=%d, sampling_time_ns=%ld>", self->pin, self->block->id, self->sample_ns);
}

// ADC initialization helper function
machine_adc_obj_t *adc_init_helper(uint32_t sampling_time, uint32_t pin) {
    // Get GPIO and check it has ADC capabilities.
    if (!IS_GPIO_VALID_ADC_PIN(pin)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid ADC Pin"));
    }

    int16_t adc_block_id = get_adc_block_id(pin);
    printf("\n ADC Block ID: %d\n", adc_block_id);

    if (adc_block_id == -1) {
        mp_raise_ValueError(MP_ERROR_TEXT("No associated ADC Block for specified pin!"));
    }

    machine_adcblock_obj_t *adc_block = adc_block_init_helper(adc_block_id, DEFAULT_ADC_BITS);
    printf("\nADC Bits: %d\n", adc_block->bits);

    machine_adc_obj_t *o = adc_block_get_pin_adc_obj(adc_block, pin);

    if (o == NULL) {
        o = adc_init_new(adc_block, pin, sampling_time);
    }

    return o;
}

// ADC(id)
STATIC mp_obj_t machine_adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // Check number of arguments
    mp_arg_check_num(n_args, n_kw, 1, 6, true);
    enum {ARG_sample_ns};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_sample_ns, MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = DEFAULT_ADC_ACQ_NS} },
    };

    // Parse the arguments.
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, all_args + 1, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    machine_pin_obj_t *adc_pin_obj = MP_OBJ_TO_PTR(all_args[0]);

    // Get user input sampling time
    uint32_t sampling_time = args[ARG_sample_ns].u_int;

    machine_adc_obj_t *o = adc_init_helper(sampling_time, adc_pin_obj->pin_addr);

    return MP_OBJ_FROM_PTR(o);
}

// block()
STATIC mp_obj_t machine_adc_block(mp_obj_t self_in) {
    const machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_FROM_PTR(self->block);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_block_obj, machine_adc_block);

// read_u16()
STATIC mp_obj_t machine_adc_read_u16(mp_obj_t self_in) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(cyhal_adc_read_u16(&(self->adc_chan_obj)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_u16_obj, machine_adc_read_u16);

// read_uv
STATIC mp_obj_t machine_adc_read_uv(mp_obj_t self_in) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(cyhal_adc_read_uv(&(self->adc_chan_obj)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_uv_obj, machine_adc_read_uv);

STATIC const mp_rom_map_elem_t machine_adc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read_u16), MP_ROM_PTR(&machine_adc_read_u16_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_uv), MP_ROM_PTR(&machine_adc_read_uv_obj) },
    { MP_ROM_QSTR(MP_QSTR_block), MP_ROM_PTR(&machine_adc_block_obj) },
};
STATIC MP_DEFINE_CONST_DICT(machine_adc_locals_dict, machine_adc_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_adc_type,
    MP_QSTR_ADC,
    MP_TYPE_FLAG_NONE,
    make_new, machine_adc_make_new,
    print, machine_adc_print,
    locals_dict, &machine_adc_locals_dict
    );
