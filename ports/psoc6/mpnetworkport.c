/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/runtime.h"
#include "py/mphal.h"
#include "pendsv.h"

#if MICROPY_PY_LWIP

#include "lwip/timeouts.h"
// #include "pico/time.h"

// Poll lwIP every 64ms by default
#define LWIP_TICK_RATE_MS 64


// fom  MTB doc
#define TM_YEAR_BASE (1900u)
#define RTC_CALLBACK_ARG (NULL)
#define RTC_INTERRUPT_PRIORITY (3u)

extern cyhal_rtc_t psoc6_rtc;


// static int32_t lwip_alarm_id = -1;

#if MICROPY_PY_NETWORK_CYW43
#include "lib/cyw43-driver/src/cyw43.h"
#include "lib/cyw43-driver/src/cyw43_country.h"
#include "lib/cyw43-driver/src/cyw43_stats.h"
// #include "hardware/irq.h"

// #define CYW43_IRQ_LEVEL                   GPIO_IRQ_LEVEL_HIGH
// #define CYW43_SHARED_IRQ_HANDLER_PRIORITY PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY

uint32_t cyw43_country_code = CYW43_COUNTRY_WORLDWIDE;
volatile int cyw43_has_pending = 0;

// static void gpio_irq_handler(void) {
// uint32_t events = gpio_get_irq_event_mask(CYW43_PIN_WL_HOST_WAKE);
// if (events & CYW43_IRQ_LEVEL) {
//     // As we use a high level interrupt, it will go off forever until it's serviced.
//     // So disable the interrupt until this is done.  It's re-enabled again by
//     // CYW43_POST_POLL_HOOK which is called at the end of cyw43_poll_func.
//     gpio_set_irq_enabled(CYW43_PIN_WL_HOST_WAKE, CYW43_IRQ_LEVEL, false);
//     cyw43_has_pending = 1;
//     pendsv_schedule_dispatch(PENDSV_DISPATCH_CYW43, cyw43_poll);
//     CYW43_STAT_INC(IRQ_COUNT);
// }
// }

void cyw43_irq_init(void) {
    printf("cyw43_irq_init\n");
    // gpio_add_raw_irq_handler_with_order_priority(IO_IRQ_BANK0, gpio_irq_handler, CYW43_SHARED_IRQ_HANDLER_PRIORITY);
    // irq_set_enabled(IO_IRQ_BANK0, true);
    // NVIC_SetPriority(PendSV_IRQn, PICO_LOWEST_IRQ_PRIORITY);
}

void cyw43_post_poll_hook(void) {
    printf("cyw43_post_poll_hook\n");
    // cyw43_has_pending = 0;
    // gpio_set_irq_enabled(CYW43_PIN_WL_HOST_WAKE, CYW43_IRQ_LEVEL, true);
}

#endif


u32_t sys_now(void) {
    printf("sys_now\n");
    // Used by LwIP
    return mp_hal_ticks_ms();
}

STATIC void lwip_poll(void) {
    // Run the lwIP internal updates
    sys_check_timeouts();
}

void lwip_lock_acquire(void) {
    printf("lwip_lock_acquire\n");
    // Prevent PendSV from running.
    pendsv_suspend();
}

void lwip_lock_release(void) {
    printf("lwip_lock_release\n");
    // Allow PendSV to run again.
    pendsv_resume();
}

// STATIC int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     pendsv_schedule_dispatch(PENDSV_DISPATCH_LWIP, lwip_poll);
//      return LWIP_TICK_RATE_MS * 1000;
// }



// STATIC void alarm_callback(void *arg, cyhal_rtc_event_t event) {
//     printf("alarm_callback\n");

//     pendsv_schedule_dispatch(PENDSV_DISPATCH_LWIP, lwip_poll);
// }


void mod_network_lwip_init(void) {
    printf("mod_network_lwip_init\n");


// // MTB
//     if (lwip_alarm_id != -1) {
//         // cyhal_rtc_enable_event(&psoc6_rtc, CYHAL_RTC_ALARM, RTC_INTERRUPT_PRIORITY, false);
//         // cy_rsllt rslt = cyhal_rtc_set_alarm_by_seconds(&psoc6_rtc, 1);
//         // printf("setting alarm : %ld\n", rslt);
//     } else {
//         cyhal_rtc_register_callback(&psoc6_rtc, alarm_callback, NULL);
//         lwip_alarm_id = 0;
//         cyhal_rtc_enable_event(&psoc6_rtc, CYHAL_RTC_ALARM, RTC_INTERRUPT_PRIORITY, true);
//     }

//     cy_rslt_t rslt = cyhal_rtc_set_alarm_by_seconds(&psoc6_rtc, 1);
//     printf("setting alarm : %ld\n", rslt);



// rp2
    // if (lwip_alarm_id != -1) {
    //     cancel_alarm(lwip_alarm_id);
    // }
    // lwip_alarm_id = add_alarm_in_us(LWIP_TICK_RATE_MS * 1000, alarm_callback, mp_const_true, true);
}

#endif // MICROPY_PY_LWIP
