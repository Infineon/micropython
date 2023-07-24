#ifndef MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
#define MICROPY_INCLUDED_MACHINE_ADCBLOCK_H

// #include "drivers/machine/psoc6_adc.h"
#define DEFAULT_ADC_BITS        12
#define ADC_BLOCK_CHANNEL_MAX   6

#define ADCBLOCK0               (0)
#define ADCBLOCK_CHANNEL_MAX    (1)
#define MAX_BLOCKS              (1)
#define MAX_CHANNELS            (6)

#include "pins.h"
typedef struct _machine_adcblock_obj_t {
    mp_obj_base_t base;
    uint8_t id;
    uint8_t bits;
    // machine_adc_obj_t *channel[ADC_BLOCK_CHANNEL_MAX];
} machine_adcblock_obj_t;

extern machine_adcblock_obj_t *adc_block[MAX_BLOCKS];
extern cyhal_adc_channel_t *adc_channels[MAX_CHANNELS];

typedef struct
{
    uint16_t block_id;
    uint16_t channel;
    uint32_t pin;
}adc_block_channel_pin_map_t;

extern const adc_block_channel_pin_map_t adc_block_pin_map[MAX_CHANNELS];


extern void machine_adcblock_init_helper(machine_adcblock_obj_t *self, uint8_t id, uint8_t bits, cyhal_adc_channel_t adc_chan_obj);
#endif // MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
