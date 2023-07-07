#ifndef MICROPY_INCLUDED_PSOC6_ADC_H
#define MICROPY_INCLUDED_PSOC6_ADC_H

// cy includes
#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"

// std includes
#include <stdint.h>

cy_rslt_t adc_init(cyhal_adc_t *obj, cyhal_gpio_t pin, const cyhal_clock_t *clk);
cy_rslt_t adc_ch_init(cyhal_adc_channel_t *adc_ch_obj, cyhal_adc_t *adc_obj, cyhal_gpio_t vplus, cyhal_gpio_t vminus, const cyhal_adc_channel_config_t *cfg);

uint16_t adc_read_u16(const cyhal_adc_channel_t *obj);

int32_t adc_read(const cyhal_adc_channel_t *obj);

int32_t adc_read_uv(const cyhal_adc_channel_t *obj);

#endif // MICROPY_INCLUDED_PSOC6_ADC_H
