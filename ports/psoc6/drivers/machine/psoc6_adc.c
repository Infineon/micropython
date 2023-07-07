/* CYHAL ADC functions */

// std includes
#include <stdbool.h>
#include <stdio.h>

// port-specific includes
#include "psoc6_adc.h"

#define VPLUS_CHANNEL_0  (P10_0)
#define MICRO_TO_MILLI_CONV_RATIO 1000u

cy_rslt_t adc_init(cyhal_adc_t *adc_obj, cyhal_gpio_t pin, const cyhal_clock_t *clk) {
    return cyhal_adc_init(adc_obj, pin, NULL);
}

cy_rslt_t adc_ch_init(cyhal_adc_channel_t *adc_ch_obj, cyhal_adc_t *adc_obj, cyhal_gpio_t vplus, cyhal_gpio_t vminus, const cyhal_adc_channel_config_t *cfg) {
    /* ADC channel configuration */
    cy_rslt_t rslt;
    rslt = cyhal_adc_channel_init_diff(adc_ch_obj, adc_obj, VPLUS_CHANNEL_0,
        CYHAL_ADC_VNEG, cfg);
    /* Initialize a channel 0 and configure it to scan the channel 0 input pin in single ended mode. */
    return rslt;

}

cy_rslt_t adc_configure(cyhal_adc_t *adc_obj, const cyhal_adc_config_t *adc_config) {
    return cyhal_adc_configure(adc_obj, adc_config);
}

uint16_t adc_read_u16(const cyhal_adc_channel_t *obj) {
    return cyhal_adc_read_u16(obj);
}

int32_t adc_read(const cyhal_adc_channel_t *obj) {
    return cyhal_adc_read(obj);

}

int32_t adc_read_uv(const cyhal_adc_channel_t *adc_ch_obj) {
    return cyhal_adc_read_uv(adc_ch_obj) / MICRO_TO_MILLI_CONV_RATIO;
}


cy_rslt_t adc_set_sample_rate(cyhal_adc_t *obj, uint32_t desired_sample_rate_hz, uint32_t *achieved_sample_rate_hz) {
    return cyhal_adc_set_sample_rate(obj, desired_sample_rate_hz, achieved_sample_rate_hz);
}
