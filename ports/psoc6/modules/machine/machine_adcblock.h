#ifndef MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
#define MICROPY_INCLUDED_MACHINE_ADCBLOCK_H

// #include "drivers/machine/psoc6_adc.h"
#define DEFAULT_ADC_BITS        12
#define ADC_BLOCK_CHANNEL_MAX   6

typedef struct _machine_adcblock_obj_t {
    mp_obj_base_t base;
    uint8_t id;
    uint8_t bits;
    machine_adc_obj_t *channel[ADC_BLOCK_CHANNEL_MAX];
} machine_adcblock_obj_t;

extern void machine_adcblock_init_helper(machine_adcblock_obj_t *self, uint8_t id, uint8_t bits, cyhal_adc_channel_t adc_chan_obj);
#endif // MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
