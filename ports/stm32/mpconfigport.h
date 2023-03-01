/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Damien P. George
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

// Options to control how MicroPython is built for this port,
// overriding defaults in py/mpconfig.h.

// board specific definitions
#include "mpconfigboard.h"
#include "mpconfigboard_common.h"

#ifndef MICROPY_CONFIG_ROM_LEVEL
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)
#endif

// memory allocation policies
#ifndef MICROPY_GC_STACK_ENTRY_TYPE
#if MICROPY_HW_SDRAM_SIZE
#define MICROPY_GC_STACK_ENTRY_TYPE uint32_t
#else
#define MICROPY_GC_STACK_ENTRY_TYPE uint16_t
#endif
#endif
#define MICROPY_ALLOC_PATH_MAX      (128)

// optimisations
#ifndef MICROPY_OPT_COMPUTED_GOTO
#define MICROPY_OPT_COMPUTED_GOTO   (1)
#endif

// Don't enable lookup cache on M0 (low RAM)
#ifndef MICROPY_OPT_MAP_LOOKUP_CACHE
#define MICROPY_OPT_MAP_LOOKUP_CACHE (__CORTEX_M > 0)
#endif

// emitters
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#ifndef MICROPY_EMIT_THUMB
#define MICROPY_EMIT_THUMB          (1)
#endif
#ifndef MICROPY_EMIT_INLINE_THUMB
#define MICROPY_EMIT_INLINE_THUMB   (1)
#endif

// Python internal features
#define MICROPY_TRACKED_ALLOC       (MICROPY_SSL_MBEDTLS)
#define MICROPY_READER_VFS          (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (0)
#define MICROPY_REPL_INFO           (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#ifndef MICROPY_FLOAT_IMPL // can be configured by each board via mpconfigboard.mk
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_FLOAT)
#endif
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_SCHEDULER_STATIC_NODES (1)
#define MICROPY_SCHEDULER_DEPTH     (8)
#define MICROPY_VFS                 (1)

// control over Python builtins
#ifndef MICROPY_PY_BUILTINS_HELP_TEXT
#define MICROPY_PY_BUILTINS_HELP_TEXT stm32_help_text
#endif
#ifndef MICROPY_PY_SYS_PLATFORM     // let boards override it if they want
#define MICROPY_PY_SYS_PLATFORM     "pyboard"
#endif
#ifndef MICROPY_PY_THREAD
#define MICROPY_PY_THREAD           (0)
#endif

// extended modules
#define MICROPY_PY_USSL_FINALISER   (MICROPY_PY_USSL)
#define MICROPY_PY_UHASHLIB_MD5     (MICROPY_PY_USSL)
#define MICROPY_PY_UHASHLIB_SHA1    (MICROPY_PY_USSL)
#define MICROPY_PY_UCRYPTOLIB       (MICROPY_PY_USSL)
#define MICROPY_PY_UOS_INCLUDEFILE  "ports/stm32/moduos.c"
#define MICROPY_PY_OS_DUPTERM       (3)
#define MICROPY_PY_UOS_DUPTERM_BUILTIN_STREAM (1)
#define MICROPY_PY_UOS_DUPTERM_STREAM_DETACHED_ATTACHED (1)
#define MICROPY_PY_UOS_SEP          (1)
#define MICROPY_PY_UOS_SYNC         (1)
#define MICROPY_PY_UOS_UNAME        (1)
#define MICROPY_PY_UOS_URANDOM      (MICROPY_HW_ENABLE_RNG)
#define MICROPY_PY_URANDOM_SEED_INIT_FUNC (rng_get())
#ifndef MICROPY_PY_UTIME
#define MICROPY_PY_UTIME            (1)
#endif
#define MICROPY_PY_UTIME_MP_HAL     (MICROPY_PY_UTIME)
#ifndef MICROPY_PY_UTIMEQ
#define MICROPY_PY_UTIMEQ           (1)
#endif
#define MICROPY_PY_LWIP_SOCK_RAW    (MICROPY_PY_LWIP)
#ifndef MICROPY_PY_MACHINE
#define MICROPY_PY_MACHINE          (1)
#ifndef MICROPY_PY_MACHINE_BITSTREAM
#define MICROPY_PY_MACHINE_BITSTREAM (1)
#endif
#define MICROPY_PY_MACHINE_PULSE    (1)
#define MICROPY_PY_MACHINE_PIN_MAKE_NEW mp_pin_make_new
#define MICROPY_PY_MACHINE_I2C      (1)
#define MICROPY_PY_MACHINE_SOFTI2C  (1)
#define MICROPY_PY_MACHINE_SPI      (1)
#define MICROPY_PY_MACHINE_SPI_MSB  (SPI_FIRSTBIT_MSB)
#define MICROPY_PY_MACHINE_SPI_LSB  (SPI_FIRSTBIT_LSB)
#define MICROPY_PY_MACHINE_SOFTSPI  (1)
#define MICROPY_PY_MACHINE_TIMER    (1)
#define MICROPY_SOFT_TIMER_TICKS_MS uwTick
#endif
#define MICROPY_HW_SOFTSPI_MIN_DELAY (0)
#define MICROPY_HW_SOFTSPI_MAX_BAUDRATE (HAL_RCC_GetSysClockFreq() / 48)
#define MICROPY_PY_UWEBSOCKET       (MICROPY_PY_LWIP)
#define MICROPY_PY_WEBREPL          (MICROPY_PY_LWIP)
#ifndef MICROPY_PY_USOCKET
#define MICROPY_PY_USOCKET          (1)
#endif
#ifndef MICROPY_PY_NETWORK
#define MICROPY_PY_NETWORK          (1)
#endif
#ifndef MICROPY_PY_ONEWIRE
#define MICROPY_PY_ONEWIRE          (1)
#endif
#ifndef MICROPY_PY_UPLATFORM
#define MICROPY_PY_UPLATFORM        (1)
#endif

// fatfs configuration used in ffconf.h
#define MICROPY_FATFS_ENABLE_LFN       (1)
#define MICROPY_FATFS_LFN_CODE_PAGE    437 /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_USE_LABEL        (1)
#define MICROPY_FATFS_RPATH            (2)
#define MICROPY_FATFS_MULTI_PARTITION  (1)

#if MICROPY_PY_PYB
extern const struct _mp_obj_module_t pyb_module;
#define PYB_BUILTIN_MODULE_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_pyb), MP_ROM_PTR(&pyb_module) },
#else
#define PYB_BUILTIN_MODULE_CONSTANTS
#endif

#if MICROPY_PY_STM
extern const struct _mp_obj_module_t stm_module;
#define STM_BUILTIN_MODULE_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_stm), MP_ROM_PTR(&stm_module) },
#else
#define STM_BUILTIN_MODULE_CONSTANTS
#endif

#if MICROPY_PY_MACHINE
#define MACHINE_BUILTIN_MODULE_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_umachine), MP_ROM_PTR(&mp_module_machine) }, \
    { MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&mp_module_machine) },
#else
#define MACHINE_BUILTIN_MODULE_CONSTANTS
#endif

#if defined(MICROPY_HW_ETH_MDC)
extern const struct _mp_obj_type_t network_lan_type;
#define MICROPY_HW_NIC_ETH                  { MP_ROM_QSTR(MP_QSTR_LAN), MP_ROM_PTR(&network_lan_type) },
#else
#define MICROPY_HW_NIC_ETH
#endif

#if MICROPY_PY_NETWORK_CYW43
extern const struct _mp_obj_type_t mp_network_cyw43_type;
#define MICROPY_HW_NIC_CYW43                { MP_ROM_QSTR(MP_QSTR_WLAN), MP_ROM_PTR(&mp_network_cyw43_type) },
#else
#define MICROPY_HW_NIC_CYW43
#endif

#if MICROPY_PY_NETWORK_WIZNET5K
extern const struct _mp_obj_type_t mod_network_nic_type_wiznet5k;
#define MICROPY_HW_NIC_WIZNET5K             { MP_ROM_QSTR(MP_QSTR_WIZNET5K), MP_ROM_PTR(&mod_network_nic_type_wiznet5k) },
#else
#define MICROPY_HW_NIC_WIZNET5K
#endif

// extra constants
#define MICROPY_PORT_CONSTANTS \
    MACHINE_BUILTIN_MODULE_CONSTANTS \
    PYB_BUILTIN_MODULE_CONSTANTS \
    STM_BUILTIN_MODULE_CONSTANTS \

#ifndef MICROPY_BOARD_NETWORK_INTERFACES
#define MICROPY_BOARD_NETWORK_INTERFACES
#endif

#define MICROPY_PORT_NETWORK_INTERFACES \
    MICROPY_HW_NIC_ETH  \
    MICROPY_HW_NIC_CYW43 \
    MICROPY_HW_NIC_WIZNET5K \
    MICROPY_BOARD_NETWORK_INTERFACES \

#define MP_STATE_PORT MP_STATE_VM

// type definitions for the specific machine

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void *)((uint32_t)(p) | 1))

#define MP_SSIZE_MAX (0x7fffffff)

// Assume that if we already defined the obj repr then we also defined these items
#ifndef MICROPY_OBJ_REPR
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned int mp_uint_t; // must be pointer size
#endif

typedef long mp_off_t;

// We have inlined IRQ functions for efficiency (they are generally
// 1 machine instruction).
//
// Note on IRQ state: you should not need to know the specific
// value of the state variable, but rather just pass the return
// value from disable_irq back to enable_irq.  If you really need
// to know the machine-specific values, see irq.h.

static inline void enable_irq(mp_uint_t state) {
    __set_PRIMASK(state);
}

static inline mp_uint_t disable_irq(void) {
    mp_uint_t state = __get_PRIMASK();
    __disable_irq();
    return state;
}

#define MICROPY_BEGIN_ATOMIC_SECTION()     disable_irq()
#define MICROPY_END_ATOMIC_SECTION(state)  enable_irq(state)

#if MICROPY_PY_THREAD
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
        if (pyb_thread_enabled) { \
            MP_THREAD_GIL_EXIT(); \
            pyb_thread_yield(); \
            MP_THREAD_GIL_ENTER(); \
        } else { \
            __WFI(); \
        } \
    } while (0);

#define MICROPY_THREAD_YIELD() pyb_thread_yield()
#else
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
        __WFI(); \
    } while (0);

#define MICROPY_THREAD_YIELD()
#endif

// For regular code that wants to prevent "background tasks" from running.
// These background tasks (LWIP, Bluetooth) run in PENDSV context.
#define MICROPY_PY_PENDSV_ENTER   uint32_t atomic_state = raise_irq_pri(IRQ_PRI_PENDSV);
#define MICROPY_PY_PENDSV_REENTER atomic_state = raise_irq_pri(IRQ_PRI_PENDSV);
#define MICROPY_PY_PENDSV_EXIT    restore_irq_pri(atomic_state);

// Prevent the "LWIP task" from running.
#define MICROPY_PY_LWIP_ENTER   MICROPY_PY_PENDSV_ENTER
#define MICROPY_PY_LWIP_REENTER MICROPY_PY_PENDSV_REENTER
#define MICROPY_PY_LWIP_EXIT    MICROPY_PY_PENDSV_EXIT

#ifndef MICROPY_PY_NETWORK_HOSTNAME_DEFAULT
#define MICROPY_PY_NETWORK_HOSTNAME_DEFAULT "mpy-stm32"
#endif

#if MICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS
// Bluetooth code only runs in the scheduler, no locking/mutex required.
#define MICROPY_PY_BLUETOOTH_ENTER uint32_t atomic_state = 0;
#define MICROPY_PY_BLUETOOTH_EXIT (void)atomic_state;
#else
// When async events are enabled, need to prevent PendSV execution racing with
// scheduler execution.
#define MICROPY_PY_BLUETOOTH_ENTER MICROPY_PY_PENDSV_ENTER
#define MICROPY_PY_BLUETOOTH_EXIT  MICROPY_PY_PENDSV_EXIT
#endif

#if defined(STM32WB)
#define MICROPY_PY_BLUETOOTH_HCI_READ_MODE MICROPY_PY_BLUETOOTH_HCI_READ_MODE_PACKET
#else
#define MICROPY_PY_BLUETOOTH_HCI_READ_MODE MICROPY_PY_BLUETOOTH_HCI_READ_MODE_BYTE
#endif

#ifndef MICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE
#define MICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE (1)
#endif

#ifndef MICROPY_PY_BLUETOOTH_ENABLE_L2CAP_CHANNELS
#define MICROPY_PY_BLUETOOTH_ENABLE_L2CAP_CHANNELS (MICROPY_BLUETOOTH_NIMBLE)
#endif

// We need an implementation of the log2 function which is not a macro
#define MP_NEED_LOG2 (1)

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

// Needed for MICROPY_PY_URANDOM_SEED_INIT_FUNC.
uint32_t rng_get(void);
