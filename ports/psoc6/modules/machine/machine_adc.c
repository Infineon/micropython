
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


bool adc_init_flag = false;

/******************************************************************************/
// MicroPython bindings for machine.ADC

const mp_obj_type_t machine_adc_type;

// Get adc block id associated to input pin
uint16_t get_adc_block_id(uint32_t pin) {
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
uint16_t get_adc_channel_number(uint32_t pin) {
    for (int i = 0; i < sizeof(adc_block_pin_map); i++)
    {
        if (pin == adc_block_pin_map[i].pin) {
            return adc_block_pin_map[i].channel;
        }
    }
    return -1;
}

// Get or construct (if not yet constructed) the associated block instance given its adc block id
// ToDo: Needs refactoring to adapt if multiple adc blocks are allowed
machine_adcblock_obj_t *get_adc_block(uint16_t adc_block_id) {
    // Check if block instance is created before. If it is, then return the adc_block associated to adc_id
    // Else create a block instance and return the same
    if (*adc_block != NULL) { // Block already created
        for (int i = 0; i < MAX_BLOCKS; i++)
        {
            return (adc_block[i]->id == adc_block_id) ? adc_block[i] : NULL;
        }
    } else { // If block not created
        machine_adcblock_obj_t *adc_block_new = mp_obj_malloc(machine_adcblock_obj_t, &machine_adcblock_type);
        adc_block_new->id = adc_block_id;
        adc_block_new->bits = DEFAULT_ADC_BITS;
        // Update the master adc_block to sync adc_block creation across classes
        adc_block[0] = adc_block_new;
        printf("\n Block created! \n");

        return adc_block_new;
    }

    return NULL;
}
// cyhal_adc_channel_t *adc_channel_obj
void  configure_adc_channel(cyhal_adc_t adc_obj, cyhal_adc_channel_t *adc_channel_obj, uint32_t pin, uint16_t adc_channel_no, uint32_t sampling_time) {
    // TODO: (Review) If not existing, now we can create the adc object
    // Configure the ADC channel !!internal
    // If channel does no exist, only then create one
    // printf("\nADC Channel OBJ add 1: %ld\n", *adc_channels[adc_channel_no]);
    // cyhal_adc_channel_t adc_channel_obj;
    if (adc_channels[adc_channel_no] != NULL) { // channel already available. So return the created channel
        printf("\nChannel already available!\n");

        // cyhal_adc_channel_t t = *adc_channels[adc_channel_no];

        adc_channel_obj = adc_channels[adc_channel_no];

        printf("\nACQ Time 1: %ld\n", (*adc_channels[adc_channel_no]).minimum_acquisition_ns);
        printf("\nACQ Time 1A: %ld\n", adc_channel_obj->minimum_acquisition_ns);

    } else { // Channel not created already. Create new one here

        printf("\nChannel is being created!\n");
        // cyhal_adc_channel_t adc_channel_obj;
        const cyhal_adc_channel_config_t channel_config =
        {
            .enable_averaging = false,
            .min_acquisition_ns = sampling_time, // TODO: if existing, can we change its configuration (sampling rate?)
            .enabled = true
        };
        // Initialize channel
        cyhal_adc_channel_init_diff(adc_channel_obj, &adc_obj, pin, CYHAL_ADC_VNEG, &channel_config);


        // Update channel created in master list
        adc_channels[adc_channel_no] = adc_channel_obj;
        printf("\nACQ Time 2: %ld\n", (*adc_channels[adc_channel_no]).minimum_acquisition_ns);
        printf("\nACQ Time 2: %ld\n", adc_channel_obj->minimum_acquisition_ns);

    }
    // return(adc_channels[adc_channel_no]);
    // return mp_const;
}


// machine_adc_print()
STATIC void machine_adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // mp_printf(print, "<ADC Pin=%u, Channel=%d, sampling_time_ns=%ld>", self->pin, self->block->ch, self->sample_ns);
    return;
}

// ADC initialization helper function
machine_adc_obj_t *adc_init_helper(uint32_t sampling_time, uint32_t pin, uint8_t bits) {
    // Get GPIO and check it has ADC capabilities.
    if (!IS_GPIO_VALID_ADC_PIN(pin)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Invalid ADC Pin"));
    }
    cyhal_adc_t adc_obj;
    // cyhal_adc_channel_t adc_channel_obj;
    // Initialize the ADC block (required only once per execution)
    if (!adc_init_flag) {
        cyhal_adc_init(&adc_obj, pin, NULL);
        adc_init_flag = true;
    }

    // TODO: get the adcblock for a given pin.
    uint16_t adc_block_id = get_adc_block_id(pin);
    printf("\n ADC Block ID: %d\n", adc_block_id);

    if (adc_block_id == -1) {
        mp_raise_ValueError(MP_ERROR_TEXT("No associated ADC Block for specified pin!"));
    }

    // TODO: get or construct (if not yet constructed) the associated block
    // instance given its adc block id
    machine_adcblock_obj_t *adc_block = get_adc_block(adc_block_id);
    printf("\nADC Bits: %d\n", adc_block[0].id);

    // Get channel number provided the pin is given. Required mapping in case channel to pin are not default.
    uint16_t adc_channel_no = get_adc_channel_number(pin);
    printf("\n Channel no. : %d\n", adc_channel_no);

    // TODO: Before creating the ADC object, check if for the associated block
    // there is already a adc object (channel) created
    // it will return null if not existing.
    cyhal_adc_channel_t *adc_channel_obj1;
    configure_adc_channel(adc_obj, adc_channel_obj1, pin, adc_channel_no, sampling_time);

    printf("\nAcquisition time A: %ld\n", adc_channel_obj1->minimum_acquisition_ns);

    // Create ADC Object
    machine_adc_obj_t *o = mp_obj_malloc(machine_adc_obj_t, &machine_adc_type);

    o->pin = pin;
    o->block = adc_block;
    o->sample_ns = sampling_time;
    o->adc_chan_obj = adc_channel_obj1;

    printf("\nAcquisition time B: %ld\n", o->adc_chan_obj->minimum_acquisition_ns);

    // TODO: (Review) Register the object in the corresponding block
    // adc_block_allocate_adc_channel(adc_block, o);

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

    machine_adc_obj_t *o = adc_init_helper(sampling_time, adc_pin_obj->pin_addr, DEFAULT_ADC_BITS);

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
    return MP_OBJ_NEW_SMALL_INT(cyhal_adc_read_u16(self->adc_chan_obj));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_u16_obj, machine_adc_read_u16);

// read_uv
STATIC mp_obj_t machine_adc_read_uv(mp_obj_t self_in) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    printf("\n WORK DUMBO \n");
    return MP_OBJ_NEW_SMALL_INT(cyhal_adc_read_uv(self->adc_chan_obj));
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
