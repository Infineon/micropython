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

#include <stdlib.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "machine_pin_phy.h"
#include "modmachine.h"
#include "mplogger.h"

#define pdm_pcm_assert_raise_val(msg, ret)   if (ret != CY_RSLT_SUCCESS) { \
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT(msg), ret); \
}

#define DEFAULT_LEFT_GAIN       0
#define DEFAULT_RIGHT_GAIN      0
#define MICROPY_HW_MAX_PDM_PCM  2

#define AUDIO_SYS_CLOCK_24_576_000_HZ      24576000u /* (Ideally 24.576 MHz) For sample rates: 8 KHz / 16 KHz / 48 KHz */
#define AUDIO_SYS_CLOCK_22_579_000_HZ      22579000u /* (Ideally 22.579 MHz) For sample rates: 22.05 KHz / 44.1 KHz */

cyhal_clock_t pdm_pcm_audio_clock;

// Constructor args
enum {
    ARG_clk,
    ARG_data,
    ARG_sample_rate,
    ARG_decimation_rate,
    ARG_bits,
    ARG_format,
    ARG_left_gain,
    ARG_right_gain,
};

typedef enum {
    BITS_16 = 16,
    BITS_18 = 18,
    BITS_20 = 20,
    BITS_24 = 24
} pdm_pcm_word_length_t;

typedef enum {
    RX,
    TX
} pdm_pcm_op_mode_t;

// To be compatible with extmod
typedef enum {
    MONO_LEFT   = CYHAL_PDM_PCM_MODE_LEFT,
    MONO_RIGHT  = CYHAL_PDM_PCM_MODE_RIGHT,
    STEREO      = CYHAL_PDM_PCM_MODE_STEREO
} format_t;

typedef enum {
    BLOCKING,
    NON_BLOCKING,
} io_mode_t;


typedef struct _machine_pdm_pcm_obj_t {
    mp_obj_base_t base;
    uint8_t pdm_pcm_id;     // Private variable in this port. ID not associated to any port pin pdm-pcm group.
    cyhal_pdm_pcm_t pdm_pcm_obj;
    uint32_t clk;
    uint32_t data;
    uint16_t pdm_pcm_op_mode; // always RX
    io_mode_t io_mode;
    format_t format;
    uint8_t bits;
    uint32_t sample_rate;
    uint8_t decimation_rate;
    int16_t left_gain;
    int16_t right_gain;
} machine_pdm_pcm_obj_t;

static void mp_machine_pdm_pcm_init_helper(machine_pdm_pcm_obj_t *self, mp_arg_val_t *args);
static machine_pdm_pcm_obj_t *mp_machine_pdm_pcm_make_new_instance(mp_int_t pdm_pcm_id);
static void mp_machine_pdm_pcm_deinit(machine_pdm_pcm_obj_t *self);


// To be preesent in extmod/pdm_pcm
MP_NOINLINE static void machine_pdm_pcm_init_helper(machine_pdm_pcm_obj_t *self, size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_sck,             MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_data,            MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_sample_rate,     MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_decimation_rate, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_bits,            MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_format,          MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_left_gain,       MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = DEFAULT_LEFT_GAIN} },
        { MP_QSTR_right_gain,      MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = DEFAULT_RIGHT_GAIN} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_pos_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_machine_pdm_pcm_init_helper(self, args);
}

static void machine_pdm_pcm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "PDM_PCM(id=%u,\n"
        "clk="MP_HAL_PIN_FMT ",\n"
        "data="MP_HAL_PIN_FMT ",\n"
        "sample_rate=%ld,\n"
        "decimation_rate=%d,\n"
        "bits=%u,\n"
        "format=%u,\n"
        "left_gain=%d,\n"
        "right_gain=%d)",
        self->pdm_pcm_id,
        mp_hal_pin_name(self->clk),
        mp_hal_pin_name(self->data),
        self->sample_rate,
        self->decimation_rate,
        self->bits,
        self->format,
        self->left_gain, self->right_gain
        );
}

// PDM_PCM.init(...)
static mp_obj_t machine_pdm_pcm_init(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_machine_pdm_pcm_deinit(self);
    machine_pdm_pcm_init_helper(self, n_pos_args - 1, pos_args + 1, kw_args);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_pdm_pcm_init_obj, 1, machine_pdm_pcm_init);

// PDM_PCM.deinit()
static mp_obj_t machine_pdm_pcm_deinit(mp_obj_t self_in) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_machine_pdm_pcm_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pdm_pcm_deinit_obj, machine_pdm_pcm_deinit);

// PDM_PCM() Constructor
static mp_obj_t machine_pdm_pcm_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t pdm_pcm_id = mp_obj_get_int(args[0]);

    machine_pdm_pcm_obj_t *self = mp_machine_pdm_pcm_make_new_instance(pdm_pcm_id);

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw_args, args + n_pos_args);
    machine_pdm_pcm_init_helper(self, n_pos_args - 1, args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

// =======================================================================================
// Private functions
// Init clock
void pdm_pcm_audio_clock_init(uint32_t audio_clock_freq_hz) {
    cyhal_clock_t pll_clock;
    cy_rslt_t result;

    static bool clock_set = false;

    result = cyhal_clock_reserve(&pll_clock, &CYHAL_CLOCK_PLL[0]);
    pdm_pcm_assert_raise_val("PLL clock reserve failed with error code: %lx", result);

    uint32_t pll_source_clock_freq_hz = cyhal_clock_get_frequency(&pll_clock);

    if (audio_clock_freq_hz != pll_source_clock_freq_hz) {
        mp_printf(&mp_plat_print, "machine.PDM_PCM: PLL0 freq is changed from %lu to %lu. This will affect all resources clock freq sourced by PLL0.\n", pll_source_clock_freq_hz, audio_clock_freq_hz);
        clock_set = false;
        pll_source_clock_freq_hz = audio_clock_freq_hz;
    }

    if (!clock_set) {
        result = cyhal_clock_set_frequency(&pll_clock,  pll_source_clock_freq_hz, NULL);
        pdm_pcm_assert_raise_val("Set PLL clock frequency failed with error code: %lx", result);
        if (!cyhal_clock_is_enabled(&pll_clock)) {
            result = cyhal_clock_set_enabled(&pll_clock, true, true);
            pdm_pcm_assert_raise_val("PLL clock enable failed with error code: %lx", result);
        }

        result = cyhal_clock_reserve(&pdm_pcm_audio_clock, &CYHAL_CLOCK_HF[1]);
        pdm_pcm_assert_raise_val("HF1 clock reserve failed with error code: %lx", result);
        result = cyhal_clock_set_source(&pdm_pcm_audio_clock, &pll_clock);
        pdm_pcm_assert_raise_val("HF1 clock sourcing failed with error code: %lx", result);

        result = cyhal_clock_set_enabled(&pdm_pcm_audio_clock, true, true);
        pdm_pcm_assert_raise_val("HF1 clock enable failed with error code: %lx", result);

        cyhal_clock_free(&pdm_pcm_audio_clock); // Check the impact while read function

        clock_set = true;
    }

    cyhal_clock_free(&pll_clock);

    cyhal_system_delay_ms(1);
}

// PDM_PCM hardware block init
static void pdm_pcm_init(machine_pdm_pcm_obj_t *self, cyhal_clock_t *clock) {
    cyhal_pdm_pcm_cfg_t config =
    {
        .sample_rate = self->sample_rate,
        .decimation_rate = self->decimation_rate,
        .mode = self->format,
        .word_length = self->bits,              /* bits */
        .left_gain = self->left_gain,           /* dB */
        .right_gain = self->right_gain,         /* dB */
    };

    cy_rslt_t result = cyhal_pdm_pcm_init(&self->pdm_pcm_obj, self->data, self->clk, &pdm_pcm_audio_clock, &config);
    assert_pin_phy_used(result);
    pdm_pcm_assert_raise_val("PDM_PCM initialisation failed with return code %lx !", result);
}

// =======================================================================================
// MPY bindings in ports/psoc6

static machine_pdm_pcm_obj_t *mp_machine_pdm_pcm_make_new_instance(mp_int_t pdm_pcm_id) {
    (void)pdm_pcm_id;
    machine_pdm_pcm_obj_t *self = NULL;
    for (uint8_t i = 0; i < MICROPY_HW_MAX_PDM_PCM; i++) {
        if (MP_STATE_PORT(machine_pdm_pcm_obj[i]) == NULL) {
            self = mp_obj_malloc(machine_pdm_pcm_obj_t, &machine_pdm_pcm_type);
            MP_STATE_PORT(machine_pdm_pcm_obj[i]) = self;
            self->pdm_pcm_id = i;
            break;
        }
    }

    if (self == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("all available pdm pcm instances are allocated"));
    }

    return self;
}

// Init helper
static void mp_machine_pdm_pcm_init_helper(machine_pdm_pcm_obj_t *self, mp_arg_val_t *args) {

    // Only RX mode supported in psoc6
    self->pdm_pcm_op_mode = RX;

    // Assign pins
    self->clk = pin_addr_by_name(args[ARG_clk].u_obj);
    self->data = pin_addr_by_name(args[ARG_data].u_obj);

    // Assign configurable parameters
    // PDM_PCM Mode
    format_t pdm_pcm_format = args[ARG_format].u_int;
    if ((pdm_pcm_format != MONO_LEFT) &&
        (pdm_pcm_format != MONO_RIGHT) &&
        (pdm_pcm_format != STEREO)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid format"));
    }
    self->format = pdm_pcm_format;

    // Check word length
    uint8_t pdm_pcm_word_length = args[ARG_bits].u_int;
    if (pdm_pcm_word_length < BITS_16 || pdm_pcm_word_length > BITS_24) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid word length"));
    }
    self->bits = args[ARG_bits].u_int;

    // Set gains
    self->left_gain = args[ARG_left_gain].u_int;
    self->left_gain = args[ARG_right_gain].u_int;

    // Set sampling and decimation rates (as given by user)
    self->sample_rate = args[ARG_sample_rate].u_int;
    self->decimation_rate = args[ARG_decimation_rate].u_int;
    // Set clock values #ToDo: To be verified for each possible values
    uint32_t audio_clock_freq_hz;
    uint32_t rate = args[ARG_sample_rate].u_int;
    if (rate == 8000 ||
        rate == 16000 ||
        rate == 48000) {
        audio_clock_freq_hz = AUDIO_SYS_CLOCK_24_576_000_HZ;
    } else if (rate == 22050 ||
               rate == 44100) {
        audio_clock_freq_hz = AUDIO_SYS_CLOCK_22_579_000_HZ;
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("rate not supported"));
    }

    pdm_pcm_audio_clock_init(audio_clock_freq_hz);
    pdm_pcm_init(self, &pdm_pcm_audio_clock);
}

static void mp_machine_pdm_pcm_deinit(machine_pdm_pcm_obj_t *self) {
    cyhal_pdm_pcm_free(&self->pdm_pcm_obj);
    MP_STATE_PORT(machine_pdm_pcm_obj[self->pdm_pcm_id]) = NULL;
}



static const mp_rom_map_elem_t machine_pdm_pcm_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_init),            MP_ROM_PTR(&machine_pdm_pcm_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),          MP_ROM_PTR(&machine_pdm_pcm_deinit_obj) },
    #if MICROPY_PY_MACHINE_PDM_PCM_FINALISER
    { MP_ROM_QSTR(MP_QSTR___del__),         MP_ROM_PTR(&machine_pdm_pcm_deinit_obj) },
    #endif

    // Constants
    // Word lengths
    { MP_ROM_QSTR(MP_QSTR_BITS_16),          MP_ROM_INT(BITS_16) },
    { MP_ROM_QSTR(MP_QSTR_BITS_18),          MP_ROM_INT(BITS_18) },
    { MP_ROM_QSTR(MP_QSTR_BITS_20),          MP_ROM_INT(BITS_20) },
    { MP_ROM_QSTR(MP_QSTR_BITS_24),          MP_ROM_INT(BITS_24) },

    // Modes
    { MP_ROM_QSTR(MP_QSTR_STEREO),          MP_ROM_INT(STEREO) },
    { MP_ROM_QSTR(MP_QSTR_MONO_LEFT),       MP_ROM_INT(MONO_LEFT) },
    { MP_ROM_QSTR(MP_QSTR_MONO_RIGHT),      MP_ROM_INT(MONO_RIGHT) },
};
MP_DEFINE_CONST_DICT(machine_pdm_pcm_locals_dict, machine_pdm_pcm_locals_dict_table);


MP_REGISTER_ROOT_POINTER(struct _machine_pdm_pcm_obj_t *machine_pdm_pcm_obj[MICROPY_HW_MAX_PDM_PCM]);


MP_DEFINE_CONST_OBJ_TYPE(
    machine_pdm_pcm_type,
    MP_QSTR_PDM_PCM,
    MP_TYPE_FLAG_NONE,
    make_new, machine_pdm_pcm_make_new,
    print, machine_pdm_pcm_print,
    locals_dict, &machine_pdm_pcm_locals_dict
    );
