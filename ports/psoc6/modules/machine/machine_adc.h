#ifndef MICROPY_INCLUDED_MACHINE_ADC_H
#define MICROPY_INCLUDED_MACHINE_ADC_H

#include "machine_adcblock.h"

// typedef struct _machine_adc_obj_t {
//     mp_obj_base_t base;
//     machine_adcblock_obj_t *block;
//     uint32_t adc_pin;
//     uint32_t sample_ns;
// } machine_adc_obj_t;

typedef struct _machine_adc_obj_t {
    mp_obj_base_t base;
    machine_adcblock_obj_t *block;
    // uint8_t  adc_block_parent_id;
    uint32_t pin;
    uint32_t sample_ns;
    cyhal_adc_channel_t adc_chan_obj;
} machine_adc_obj_t;

extern machine_adc_obj_t *adc_init_helper(uint32_t sampling_time, uint32_t pin1, uint8_t bits);

#endif // MICROPY_INCLUDED_MACHINE_ADC_H
