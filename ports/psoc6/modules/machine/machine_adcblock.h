#ifndef MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
#define MICROPY_INCLUDED_MACHINE_ADCBLOCK_H

#include "drivers/machine/psoc6_adc.h"
#define DEFAULT_ADC_BITS    12

typedef struct _machine_adcblock_obj_t {
    mp_obj_base_t base;
    uint8_t adc_id;
    uint8_t bits;
    uint8_t ch;
    uint32_t adc_pin;
    bool flag;
    cyhal_adc_channel_t adc_chan_obj;
} machine_adcblock_obj_t;

typedef struct _ch_pin_map_t {
    uint8_t ch;
    uint32_t pin;
} ch_pin_map_t;

extern void machine_adcblock_init_helper(machine_adcblock_obj_t *self, uint8_t id, uint8_t bits, cyhal_adc_channel_t adc_chan_obj);
#endif // MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
