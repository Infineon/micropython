/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022-2024 Infineon Technologies AG
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

// std includes
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>


// MTB includes
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyhal.h"

// FreeRTOS header file
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


// micropython includes
#include "genhdr/mpversion.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "shared/readline/readline.h"
#include "shared/runtime/pyexec.h"
#include "extmod/modnetwork.h"

#include "modmachine.h"
#include "machine_pin_phy.h"

#if MICROPY_PY_NETWORK
#include "cybsp_wifi.h"
#include "cy_wcm.h"
#endif

#if MICROPY_PY_LWIP
#include "lwip/init.h"
#include "lwip/apps/mdns.h"
#endif

// port-specific includes
#include "mplogger.h"

///////////////////////////////////////////////////////////////
///// Inserting code used by switching power modes ////////////

/* Constants to define LONG and SHORT presses on User Button (x10 = ms) */
#define QUICK_PRESS_COUNT       2u      /* 20 ms < press < 200 ms */
#define SHORT_PRESS_COUNT       20u     /* 200 ms < press < 2 sec */
#define LONG_PRESS_COUNT        200u    /* press > 2 sec */

/* PWM LED frequency constants (in Hz) */
#define PWM_FAST_FREQ_HZ        10
#define PWM_SLOW_FREQ_HZ        3
#define PWM_DIM_FREQ_HZ         100

/* PWM Duty cycles (Active Low, in %) */
#define PWM_50P_DUTY_CYCLE      50.0f
#define PWM_10P_DUTY_CYCLE      90.0f
#define PWM_100P_DUTY_CYCLE     0.0f

/* Clock frequency constants (in Hz) */
#define CLOCK_50_MHZ            50000000u
#define CLOCK_100_MHZ           100000000u

/* Glitch delays */
#define SHORT_GLITCH_DELAY_MS   10u     /* in ms */
#define LONG_GLITCH_DELAY_MS    100u    /* in ms */

/* User button press delay*/
#define USER_BTN_PRESS_DELAY    10u     /* in ms */

#define led_conf_on_bright()    cyhal_pwm_set_duty_cycle(&pwm, PWM_100P_DUTY_CYCLE, PWM_DIM_FREQ_HZ)
#define led_conf_on_dimmed()    cyhal_pwm_set_duty_cycle(&pwm, PWM_10P_DUTY_CYCLE, PWM_DIM_FREQ_HZ)
#define led_conf_off()
#define led_conf_blink_fast()   cyhal_pwm_set_duty_cycle(&pwm, PWM_50P_DUTY_CYCLE, PWM_FAST_FREQ_HZ)
#define led_conf_blink_slow()   cyhal_pwm_set_duty_cycle(&pwm, PWM_50P_DUTY_CYCLE, PWM_SLOW_FREQ_HZ)

typedef enum
{
    SWITCH_NO_EVENT     = 0u,
    SWITCH_QUICK_PRESS  = 1u,
    SWITCH_SHORT_PRESS  = 2u,
    SWITCH_LONG_PRESS   = 3u,
} en_switch_event_t;

extern uint8_t selected_event;

/*****************************************************************************
* Function Prototypes
********************************************************************************/
en_switch_event_t get_switch_event(void);

/* Power callbacks */
bool pwm_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg);
bool clk_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg);


/*******************************************************************************
* Global Variables
********************************************************************************/
/* HAL Objects */
// cyhal_pwm_t pwm;
// cyhal_clock_t system_clock;

// #define BUTLEDPWMODE_TASK_STACK_SIZE_BYTES  (4096)
// #define BUTLEDPWMODE_TASK_PRIORITY          (3U)

// TaskHandle_t button_led_pwm_mode_task_handle;

// void button_led_pwm_mode_task()
// {
//     cy_rslt_t result;

//     /* Initialize the User Button */
//     cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
//     /* Enable the GPIO interrupt to wake-up the device */
//     cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, CYHAL_ISR_PRIORITY_DEFAULT, true);

//     /* Initialize the PWM to control LED brightness */
//     cyhal_pwm_init(&pwm, CYBSP_USER_LED, NULL);
//     cyhal_pwm_set_duty_cycle(&pwm, PWM_50P_DUTY_CYCLE, PWM_FAST_FREQ_HZ);
//     cyhal_pwm_start(&pwm);

//     /* Reserve, take ownership of, the specified clock instance*/
//     result = cyhal_clock_reserve(&system_clock, &CYHAL_CLOCK_FLL);

// // If the clock is not already enabled, enable it
//     if (!cyhal_clock_is_enabled(&system_clock))
//     {
//         result = cyhal_clock_set_enabled(&system_clock, true, true);
//     }

//     (void)result;

//     /* Callback declaration for Power Modes */
//     cyhal_syspm_callback_data_t pwm_callback = {pwm_power_callback,             /* Callback function */
//                                                (cyhal_syspm_callback_state_t)
//                                                (CYHAL_SYSPM_CB_CPU_SLEEP |
//                                                 CYHAL_SYSPM_CB_CPU_DEEPSLEEP |
//                                                 CYHAL_SYSPM_CB_SYSTEM_NORMAL |
//                                                 CYHAL_SYSPM_CB_SYSTEM_LOW),     /* Power States supported */
//                                                (cyhal_syspm_callback_mode_t)
//                                                (CYHAL_SYSPM_CHECK_FAIL),        /* Modes to ignore */
//                                                 NULL,                           /* Callback Argument */
//                                                 NULL};                          /* For internal use */

//     cyhal_syspm_callback_data_t clk_callback = {clk_power_callback,             /* Callback function */
//                                                (cyhal_syspm_callback_state_t)
//                                                (CYHAL_SYSPM_CB_CPU_SLEEP |
//                                                 CYHAL_SYSPM_CB_CPU_DEEPSLEEP |
//                                                 CYHAL_SYSPM_CB_SYSTEM_NORMAL |
//                                                 CYHAL_SYSPM_CB_SYSTEM_LOW),     /* Power States supported */
//                                                (cyhal_syspm_callback_mode_t)
//                                                (CYHAL_SYSPM_CHECK_READY |
//                                                 CYHAL_SYSPM_CHECK_FAIL),        /* Modes to ignore */
//                                                 NULL,                           /* Callback Argument */
//                                                 NULL};                          /* For internal use */

//     /* Initialize the System Power Management */
//     cyhal_syspm_init();

//     /* Power Management Callback registration */
//     cyhal_syspm_register_callback(&pwm_callback);
//     cyhal_syspm_register_callback(&clk_callback);


// //    button_led_pwm_task_init();
//     // const TickType_t xDelay = 100 / portTICK_PERIOD_MS;

//     for(;;)
//     {
//         switch (get_switch_event())
//         {
//             case SWITCH_QUICK_PRESS:
//                 /* Check if the device is in System Low Power state */
//                 if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
//                 {
//                     /* Switch to System Normal Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_NORMAL);
//                 }
//                 else
//                 {
//                     /* Switch to System Low Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_LOW);
//                 }
//                 break;

//             case SWITCH_SHORT_PRESS:
//                 /* Go to sleep */
//                 cyhal_syspm_sleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 vTaskSuspend(NULL);
//                 break;

//             case SWITCH_LONG_PRESS:
//                 /* Go to deep sleep */
//                 cyhal_syspm_deepsleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 break;

//             default:
//                 break;
//         }
// //        taskYIELD();
// //        vTaskSuspend(NULL);
// //        vTaskDelay(xDelay);
//     }

// }

extern en_switch_event_t get_switch_event(void);
// en_switch_event_t get_switch_event(void)
// {
//     en_switch_event_t event = SWITCH_NO_EVENT;
//     uint32_t pressCount = 0;

//     /* Check if User button is pressed */
//     while (cyhal_gpio_read(CYBSP_USER_BTN) == CYBSP_BTN_PRESSED)
//     {
//         /* Wait for 10 ms */
//         cyhal_system_delay_ms(USER_BTN_PRESS_DELAY);

//         /* Increment counter. Each count represents 10 ms */
//         pressCount++;
//     }

//     /* Check for how long the button was pressed */
//     if (pressCount > LONG_PRESS_COUNT)
//     {
//         event = SWITCH_LONG_PRESS;
//     }
//     else if (pressCount > SHORT_PRESS_COUNT)
//     {
//         event = SWITCH_SHORT_PRESS;
//     }
//     else if (pressCount > QUICK_PRESS_COUNT)
//     {
//         event = SWITCH_QUICK_PRESS;
//     }

//     /* Add a delay to avoid glitches */
//     cyhal_system_delay_ms(SHORT_GLITCH_DELAY_MS);

//     return event;
// }

TaskHandle_t mpy_task_handle;

// bool pwm_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg)
// {
//     (void) arg;

//     /* Stop the PWM before applying any changes */
//     cyhal_pwm_stop(&pwm);

//     if (mode == CYHAL_SYSPM_BEFORE_TRANSITION)
//     {
//         if (state == CYHAL_SYSPM_CB_CPU_SLEEP)
//         {
//             /* Check if the device is in Low Power Mode */
//             if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
//             {
//                 /* Before going to Low Power Sleep Mode, set LED brightness to 10% */
//                 led_conf_on_dimmed();
//             }
//             else
//             {
//                 /* Before going to Normal Power Sleep Mode, set LED brightness to 100% */
//                 led_conf_on_bright();
//             }

//             /* Restart the PWM */
//             cyhal_pwm_start(&pwm);

//         }
//     }
//     else if (mode == CYHAL_SYSPM_AFTER_TRANSITION)
//     {
//         switch (state)
//         {
// //            case CYHAL_SYSPM_CB_CPU_SLEEP:
// //			/* Check if the device is in Low Power Mode */
// //			if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
// //			{
// //				/* Before going to Low Power Sleep Mode, set LED brightness to 10% */
// ////				cyhal_pwm_set_duty_cycle(&pwm, PWM_100P_DUTY_CYCLE, PWM_DIM_FREQ_HZ);
// //				led_conf_on_dimmed();
// //			}
// //			else
// //			{
// //				/* Before going to Normal Power Sleep Mode, set LED brightness to 100% */
// ////				cyhal_pwm_set_duty_cycle(&pwm, PWM_10P_DUTY_CYCLE, PWM_DIM_FREQ_HZ);
// //				led_conf_on_bright();
// //			}
// //			break;

//             case CYHAL_SYSPM_CB_CPU_SLEEP:
//             // {
//                      //      BaseType_t xYieldRequired = pdFALSE;
//                      //      xYieldRequired = xTaskResumeFromISR( mpy_task_handle );
//                      //      portYIELD_FROM_ISR( xYieldRequired );
//             // }
//              case CYHAL_SYSPM_CB_CPU_DEEPSLEEP:
//                 /* Check if the device is in Low Power Mode */
//                 if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
//                 {
//                     /* After waking up, set the slow blink pattern */
//                      led_conf_blink_slow();
//                 }
//                 else
//                 {
//                     /* After waking up, set the fast blink pattern */
//                      led_conf_blink_fast();
//                 }
//                 break;

//             case CYHAL_SYSPM_CB_SYSTEM_NORMAL:
//                 /* Set fast blinking rate when in Normal Power state*/
//              led_conf_blink_fast();
//                 break;

//             case CYHAL_SYSPM_CB_SYSTEM_LOW:
//                 /* Set slow blinking rate when in Low Power state */
//                 led_conf_blink_slow();
//                 break;

//             default:
//                 break;
//         }

//         /* Restart the PWM */
//         cyhal_pwm_start(&pwm);
//     }

//     return true;
// }


// void switch_power_modes_loop() {
//         switch (get_switch_event())
//         {
//             case SWITCH_QUICK_PRESS:
//                 /* Check if the device is in System Low Power state */
//                 if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
//                 {
//                     /* Switch to System Normal Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_NORMAL);
//                 }
//                 else
//                 {
//                     /* Switch to System Low Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_LOW);
//                 }
//                 break;

//             case SWITCH_SHORT_PRESS:
//                 /* Go to sleep */
//                 cyhal_syspm_sleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 vTaskSuspend(NULL);
//                 break;

//             case SWITCH_LONG_PRESS:
//                 /* Go to deep sleep */
//                 cyhal_syspm_deepsleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 break;

//             default:
//                 break;
//         }
// }

// bool clk_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg)
// {
//     (void) arg;

//     if (mode == CYHAL_SYSPM_AFTER_TRANSITION)
//     {
//         switch (state)
//         {
//             case CYHAL_SYSPM_CB_SYSTEM_NORMAL:
//                 /* Set the system clock to 100 MHz */
//                 // cyhal_clock_set_frequency(&system_clock, CLOCK_100_MHZ, NULL);
//                 break;

//             case CYHAL_SYSPM_CB_CPU_SLEEP:
//                 {
//                     BaseType_t xYieldRequired = pdFALSE;
//                     xYieldRequired = xTaskResumeFromISR( mpy_task_handle );
//                     portYIELD_FROM_ISR( xYieldRequired );
//                 }
//                 break;

//             case CYHAL_SYSPM_CB_CPU_DEEPSLEEP:
//                 {
//                     BaseType_t xYieldRequired = pdFALSE;
//                     xYieldRequired = xTaskResumeFromISR( mpy_task_handle );
//                     portYIELD_FROM_ISR( xYieldRequired );
//                 }
//                 break;

//             case CYHAL_SYSPM_CB_SYSTEM_LOW:
//                 // /* Set the System Clock to 50 MHz */
//                 // cyhal_clock_set_frequency(&system_clock, CLOCK_50_MHZ, NULL);
//                 break;

//             default:
//                 break;
//         }
//         // if (state == CYHAL_SYSPM_CB_SYSTEM_NORMAL)
//         // {
//         //     /* Set the system clock to 100 MHz */
//         //     cyhal_clock_set_frequency(&system_clock, CLOCK_100_MHZ, NULL);
//         // }
//         // else if (state == CYHAL_SYSPM_CB_CPU_SLEEP)
//         // {
//         //         BaseType_t xYieldRequired = pdFALSE;
//              //              xYieldRequired = xTaskResumeFromISR( mpy_task_handle );
//              //              portYIELD_FROM_ISR( xYieldRequired );
//         // } else if (state == CYHAL_SYSPM_CB_CPU_DEEPSLEEP) {
//         //         BaseType_t xYieldRequired = pdFALSE;
//              //              xYieldRequired = xTaskResumeFromISR( mpy_task_handle );
//              //              portYIELD_FROM_ISR( xYieldRequired );
//         // }
//     }
//     else if (mode == CYHAL_SYSPM_BEFORE_TRANSITION)
//     {
//         switch (state)
//         {
//         case CYHAL_SYSPM_CB_SYSTEM_LOW:
//             /* Set the System Clock to 50 MHz */
//             // mp_printf(&mp_plat_print, "bef low\n");
//             // cyhal_clock_set_frequency(&system_clock, CLOCK_50_MHZ, NULL);
//         break;

//         case CYHAL_SYSPM_CB_CPU_DEEPSLEEP:
//             vTaskSuspend(mpy_task_handle);
//         default:
//             break;
//         }
//         if (state == CYHAL_SYSPM_CB_SYSTEM_LOW)
//         {
//             /* Set the System Clock to 50 MHz */
//             // cyhal_clock_set_frequency(&system_clock, CLOCK_50_MHZ, NULL);
//         }
//     }

//     return true;
// }

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#define MPY_TASK_STACK_SIZE                    (4096)
#define MPY_TASK_PRIORITY                      (3U)

void mpy_task();

typedef enum {
    BOOT_MODE_NORMAL,
    BOOT_MODE_SAFE
} boot_mode_t;

#if MICROPY_ENABLE_GC
extern uint8_t __StackTop, __StackLimit;
__attribute__((section(".bss"))) static char gc_heap[MICROPY_GC_HEAP_SIZE];
#endif

extern void mod_rtc_init(void);
extern void time_init(void);
extern void os_init(void);
extern void network_init(void);
extern void network_deinit(void);

boot_mode_t check_boot_mode(void) {
    boot_mode_t boot_mode;

    // initialize user LED
    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
        CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    // initialize user button
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
        CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    // Added 5ms delay to allow bypass capacitor connected to the user button without external pull-up to charge.
    cyhal_system_delay_ms(5);

    if (cyhal_gpio_read(CYBSP_USER_BTN) == CYBSP_BTN_PRESSED) {
        // Blink LED twice to indicate safe boot mode was entered
        for (int i = 0; i < 4; i++)
        {
            cyhal_gpio_toggle(CYBSP_USER_LED);
            cyhal_system_delay_ms(500); // delay in millisecond
        }
        boot_mode = BOOT_MODE_SAFE;
        mp_printf(&mp_plat_print, "- DEVICE IS IN SAFE BOOT MODE -\n");
    } else { // normal boot mode
        boot_mode = BOOT_MODE_NORMAL;
    }
    // free the user LED and user button
    cyhal_gpio_free(CYBSP_USER_BTN);
    cyhal_gpio_free(CYBSP_USER_LED);

    return boot_mode;
}

int main(int argc, char **argv) {
    // Initialize the device and board peripherals
    cy_rslt_t result = cybsp_init();
    // TODO: the printing is not ready yet.
    if (result != CY_RSLT_SUCCESS) {
        mp_raise_ValueError(MP_ERROR_TEXT("cybsp_init failed !\n"));
    }

    // Initialize retarget-io to use the debug UART port
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        // TODO: the printing is not guaranteed to be ready yet.`??
        mp_raise_ValueError(MP_ERROR_TEXT("cy_retarget_io_init failed !\n"));
    }

    /////////
    // __enable_irq();
    ////////

    xTaskCreate(mpy_task, "MicroPython task", MPY_TASK_STACK_SIZE, NULL, MPY_TASK_PRIORITY, &mpy_task_handle);
    // xTaskCreate(button_led_pwm_mode_task, "Butt LED PWM Task", BUTLEDPWMODE_TASK_STACK_SIZE_BYTES,
    //     NULL, BUTLEDPWMODE_TASK_PRIORITY, &button_led_pwm_mode_task_handle);

    vTaskStartScheduler();

    // Should never get here
    CY_ASSERT(0);
    return 0;
}

void mpy_task() {
    #if MICROPY_ENABLE_GC
    mp_stack_set_top(&__StackTop);
    // mp_stack_set_limit((mp_uint_t)&__StackTop - (mp_uint_t)&__StackLimit);
    mp_stack_set_limit((mp_uint_t)&__StackLimit);
    gc_init(&gc_heap[0], &gc_heap[MP_ARRAY_SIZE(gc_heap)]);
    #endif

    // Initialize modules. Or to be redone after a reset and therefore to be placed next to machine_init below ?
    os_init();
    // time_init(); // TODO: This does not allow to enter deep sleep mode

soft_reset:
    mod_rtc_init();
    mp_init();

    // ANSI ESC sequence for clear screen. Refer to  https://stackoverflow.com/questions/517970/how-to-clear-the-interpreter-console
    mp_printf(&mp_plat_print, "\033[H\033[2J");

    // indicate in REPL console when debug mode is selected
    mplogger_print("\n...LOGGER DEBUG MODE...\n\n");

    readline_init0();
    machine_init();
    #if MICROPY_PY_NETWORK
    network_init();
    mod_network_init();
    #endif

    #if MICROPY_VFS
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));

    #if MICROPY_VFS_LFS2
    pyexec_frozen_module("vfs_lfs2.py", false);
    #elif MICROPY_VFS_FAT
    pyexec_frozen_module("vfs_fat.py", false);
    #endif

    #endif

    if (check_boot_mode() == BOOT_MODE_NORMAL) {
        // Execute user scripts.
        int ret = pyexec_file_if_exists("/boot.py");

        if (ret & PYEXEC_FORCED_EXIT) {
            goto soft_reset;
        }

        if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
            ret = pyexec_file_if_exists("/main.py");

            if (ret & PYEXEC_FORCED_EXIT) {
                goto soft_reset;
            }
        }
    }

    __enable_irq();

//    cy_rslt_t result;

    /* Initialize the User Button */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    /* Enable the GPIO interrupt to wake-up the device */
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, CYHAL_ISR_PRIORITY_DEFAULT, true);

    /* Initialize the PWM to control LED brightness */
    // cyhal_pwm_init(&pwm, CYBSP_USER_LED, NULL);
    // cyhal_pwm_set_duty_cycle(&pwm, PWM_50P_DUTY_CYCLE, PWM_FAST_FREQ_HZ);
    // cyhal_pwm_start(&pwm);

    /* Reserve, take ownership of, the specified clock instance*/
    // result =
//     cyhal_clock_reserve(&system_clock, &CYHAL_CLOCK_FLL);

// // If the clock is not already enabled, enable it
//     if (!cyhal_clock_is_enabled(&system_clock))
//     {
//         // result =
//         cyhal_clock_set_enabled(&system_clock, true, true);
//     }

    // (void)result;

    // /* Callback declaration for Power Modes */
    // cyhal_syspm_callback_data_t pwm_callback = {pwm_power_callback,             /* Callback function */
    //                                            (cyhal_syspm_callback_state_t)
    //                                            (CYHAL_SYSPM_CB_CPU_SLEEP |
    //                                             CYHAL_SYSPM_CB_CPU_DEEPSLEEP |
    //                                             CYHAL_SYSPM_CB_SYSTEM_NORMAL |
    //                                             CYHAL_SYSPM_CB_SYSTEM_LOW),     /* Power States supported */
    //                                            (cyhal_syspm_callback_mode_t)
    //                                            (CYHAL_SYSPM_CHECK_FAIL),        /* Modes to ignore */
    //                                             NULL,                           /* Callback Argument */
    //                                             NULL};                          /* For internal use */

    // cyhal_syspm_callback_data_t clk_callback = {clk_power_callback,             /* Callback function */
    //                                            (cyhal_syspm_callback_state_t)
    //                                            (CYHAL_SYSPM_CB_CPU_SLEEP |
    //                                             CYHAL_SYSPM_CB_CPU_DEEPSLEEP |
    //                                             CYHAL_SYSPM_CB_SYSTEM_NORMAL |
    //                                             CYHAL_SYSPM_CB_SYSTEM_LOW),     /* Power States supported */
    //                                            (cyhal_syspm_callback_mode_t)
    //                                            (CYHAL_SYSPM_CHECK_READY |
    //                                             CYHAL_SYSPM_CHECK_FAIL),        /* Modes to ignore */
    //                                             NULL,                           /* Callback Argument */
    //                                             NULL};                          /* For internal use */

    /* Initialize the System Power Management */
    // cyhal_syspm_init();

    /* Power Management Callback registration */
    // cyhal_syspm_register_callback(&pwm_callback);
    // cyhal_syspm_register_callback(&clk_callback);


//    button_led_pwm_task_init();
    // const TickType_t xDelay = 100 / portTICK_PERIOD_MS;

//     for(;;)
//     {
//         switch (get_switch_event())
//         {
//             case SWITCH_QUICK_PRESS:
//                 /* Check if the device is in System Low Power state */
//                 if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
//                 {
//                     /* Switch to System Normal Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_NORMAL);
//                 }
//                 else
//                 {
//                     /* Switch to System Low Power state */
//                     cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_LOW);
//                 }
//                 break;

//             case SWITCH_SHORT_PRESS:
//                 /* Go to sleep */
//                 cyhal_syspm_sleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 vTaskSuspend(NULL);
//                 break;

//             case SWITCH_LONG_PRESS:
//                 /* Go to deep sleep */
//                 cyhal_syspm_deepsleep();

//                 /* Wait a bit to avoid glitches from the button press */
//                 cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
//                 break;

//             default:
//                 break;
//         }
// //        taskYIELD();
// //        vTaskSuspend(NULL);
// //        vTaskDelay(xDelay);
    // }

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
                break;
            }
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }

        ////////

        // switch_power_modes_loop();

        // switch (get_switch_event())
        // {
        //     case SWITCH_QUICK_PRESS:
        //         /* Check if the device is in System Low Power state */
        //         if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_LOW)
        //         {
        //             /* Switch to System Normal Power state */
        //             cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_NORMAL);
        //         }
        //         else
        //         {
        //             /* Switch to System Low Power state */
        //             cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_LOW);
        //         }
        //         break;

        //     case SWITCH_SHORT_PRESS:
        //         /* Go to sleep */
        //         cyhal_syspm_sleep();

        //         /* Wait a bit to avoid glitches from the button press */
        //         cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
        //         vTaskSuspend(NULL);
        //         break;

        //     case SWITCH_LONG_PRESS:
        //         /* Go to deep sleep */
        //         cyhal_syspm_deepsleep();

        //         /* Wait a bit to avoid glitches from the button press */
        //         cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);
        //         break;

        //     default:
        //         break;
        // }
        ////////
    }

    mp_printf(&mp_plat_print, "MPY: soft reboot\n");
    #if MICROPY_PY_NETWORK
    mod_network_deinit();
    network_deinit();
    #endif
    gc_sweep_all();
    mp_deinit();

    goto soft_reset;
}

// TODO: to be implemented
void nlr_jump_fail(void *val) {
    mplogger_print("nlr_jump_fail\n");

    mp_printf(&mp_plat_print, "FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));

    for (;;) {
        __BKPT(0);
    }
}
