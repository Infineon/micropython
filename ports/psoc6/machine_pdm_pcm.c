/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Infineon Technologies AG
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
#include "py/stream.h"
#include "py/mphal.h"
#include "machine_pin_phy.h"
#include "modmachine.h"
#include "mplogger.h"

#if MICROPY_PY_MACHINE_PDM_PCM
#include "machine_pdm_pcm.h"

cyhal_clock_t pdm_pcm_audio_clock;

#if MICROPY_PY_MACHINE_PDM_PCM_RING_BUF
static void ringbuf_init(ring_buf_t *rbuf, uint8_t *buffer, size_t size) {
    rbuf->buffer = buffer;
    rbuf->size = size;
    rbuf->head = 0;
    rbuf->tail = 0;
}

static bool ringbuf_push(ring_buf_t *rbuf, uint8_t data) {
    size_t next_tail = (rbuf->tail + 1) % rbuf->size;

    if (next_tail != rbuf->head) {
        rbuf->buffer[rbuf->tail] = data;
        rbuf->tail = next_tail;
        return true;
    }
    // full
    return false;
}

static bool ringbuf_pop(ring_buf_t *rbuf, uint8_t *data) {
    if (rbuf->head == rbuf->tail) {
        // empty
        return false;
    }
    *data = rbuf->buffer[rbuf->head];
    rbuf->head = (rbuf->head + 1) % rbuf->size;
    return true;
}

static size_t ringbuf_available_data(ring_buf_t *rbuf) {
    return (rbuf->tail - rbuf->head + rbuf->size) % rbuf->size;
}

static size_t ringbuf_available_space(ring_buf_t *rbuf) {
    return rbuf->size - ringbuf_available_data(rbuf) - 1;
}

static uint32_t fill_appbuf_from_ringbuf(machine_pdm_pcm_obj_t *self, mp_buffer_info_t *appbuf) {
    uint32_t num_bytes_copied_to_appbuf = 0;
    uint8_t *app_p = (uint8_t *)appbuf->buf;
    uint8_t appbuf_sample_size_in_bytes = (self->bits == 16? 2 : 4) * (self->format == STEREO ? 2: 1); // !J: Why only 16 or 32?
    uint32_t num_bytes_needed_from_ringbuf = appbuf->len * (PDM_PCM_RX_FRAME_SIZE_IN_BYTES / appbuf_sample_size_in_bytes);
    uint8_t discard_byte;
    while (num_bytes_needed_from_ringbuf) {

        uint8_t f_index = get_frame_mapping_index(self->bits, self->format);

        for (uint8_t i = 0; i < PDM_PCM_RX_FRAME_SIZE_IN_BYTES; i++) {
            int8_t r_to_a_mapping = pdm_pcm_frame_map[f_index][i];
            if (r_to_a_mapping != -1) {
                if (self->io_mode == BLOCKING) {
                    // poll the ringbuf until a sample becomes available,  copy into appbuf using the mapping transform
                    while (ringbuf_pop(&self->ring_buffer, app_p + r_to_a_mapping) == false) {
                        ;
                    }
                    num_bytes_copied_to_appbuf++;
                } else {
                    return 0;  // should never get here (non-blocking mode does not use this function)
                }
            } else { // r_a_mapping == -1
                // discard unused byte from ring buffer
                if (self->io_mode == BLOCKING) {
                    // poll the ringbuf until a sample becomes available
                    while (ringbuf_pop(&self->ring_buffer, &discard_byte) == false) {
                        ;
                    }
                } else {
                    return 0;  // should never get here (non-blocking mode does not use this function)
                }
            }
            num_bytes_needed_from_ringbuf--;
        }
        app_p += appbuf_sample_size_in_bytes;
    }
exit:
    return num_bytes_copied_to_appbuf;
}

#endif // MICROPY_PY_MACHINE_PDM_PCM_RING_BUF

/*========================================================================================================================*/
// PDM_PCM higher level MPY functions (extmod/machine_pdm_pcm.c)

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

// PDM_PCM(...) Constructor
static mp_obj_t machine_pdm_pcm_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t pdm_pcm_id = mp_obj_get_int(args[0]);

    machine_pdm_pcm_obj_t *self = mp_machine_pdm_pcm_make_new_instance(pdm_pcm_id);

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw_args, args + n_pos_args);
    machine_pdm_pcm_init_helper(self, n_pos_args - 1, args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

// PDM_PCM.init(...)
static mp_obj_t machine_pdm_pcm_init(mp_obj_t self_in) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_machine_pdm_pcm_init(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pdm_pcm_init_obj, machine_pdm_pcm_init);

// PDM_PCM.deinit()
static mp_obj_t machine_pdm_pcm_deinit(mp_obj_t self_in) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_machine_pdm_pcm_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pdm_pcm_deinit_obj, machine_pdm_pcm_deinit);

// =======================================================================================
// Port Private functions for DMA support (ports/psoc6)

static inline void _dma_buff_init(machine_pdm_pcm_obj_t *self) {
    for (uint32_t i = 0; i < SIZEOF_DMA_BUFFER; i++) {
        self->dma_active_buffer[i] = 0;
        self->dma_processing_buffer[i] = 0;
    }

    self->dma_active_buf_p = self->dma_active_buffer;
    self->dma_processing_buf_p = self->dma_processing_buffer;
}

static inline void _dma_swap_active_dmabuf(machine_pdm_pcm_obj_t *self) {
    uint32_t *temp = self->dma_active_buf_p;
    self->dma_active_buf_p = self->dma_processing_buf_p;
    self->dma_processing_buf_p = temp;
}

static void _dma_copy_from_dmabuf_to_ringbuf(machine_pdm_pcm_obj_t *self) {
    uint8_t dma_sample_size_in_bytes = (self->bits == 16? 2 : 4) * (self->format == STEREO ? 2: 1);
    uint8_t *dma_buff_p = (uint8_t *)self->dma_processing_buf_p;
    uint32_t num_bytes_needed_from_ringbuf = SIZEOF_DMA_BUFFER_IN_BYTES * (PDM_PCM_RX_FRAME_SIZE_IN_BYTES / dma_sample_size_in_bytes);

    // when space exists, copy samples into ring buffer
    if (ringbuf_available_space(&self->ring_buffer) >= num_bytes_needed_from_ringbuf) {
        uint8_t f_index = get_frame_mapping_index(self->bits, self->format);
        uint32_t i = 0;
        while (i < SIZEOF_DMA_BUFFER_IN_BYTES) {
            for (uint8_t j = 0; j < PDM_PCM_RX_FRAME_SIZE_IN_BYTES; j++) {
                int8_t r_to_a_mapping = pdm_pcm_frame_map[f_index][j];
                if (r_to_a_mapping != -1) {
                    ringbuf_push(&self->ring_buffer, dma_buff_p[i]);
                    i++;
                } else { // r_a_mapping == -1
                    ringbuf_push(&self->ring_buffer, -1);
                }
            }
        }
    }
}

// =======================================================================================
// PDM_PCM low-level abstracted functions (ports/psoc6)

// Init hardware block
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

// Initialize audio clock
void pdm_pcm_audio_clock_init(uint32_t audio_clock_freq_hz) {
    cyhal_clock_t pll_clock;
    cy_rslt_t result;

    static bool clock_set = false;

    result = cyhal_clock_reserve(&pll_clock, &CYHAL_CLOCK_PLL[0]);
    pdm_pcm_assert_raise_val("PLL clock reserve failed with error code: %lx", result);

    uint32_t pll_source_clock_freq_hz = cyhal_clock_get_frequency(&pll_clock);

    if (audio_clock_freq_hz != pll_source_clock_freq_hz) {
        mp_printf(&mp_plat_print, "machine.PDM_PCM: PLL0 freq is changed to %lu. This will affect all resources clock freq sourced by PLL0.\n", audio_clock_freq_hz);
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

// Set PDM_PCM async mode to DMA
static void pdm_pcm_set_async_mode_dma(machine_pdm_pcm_obj_t *self) {
    cy_rslt_t result = cyhal_pdm_pcm_set_async_mode(&self->pdm_pcm_obj, CYHAL_ASYNC_DMA, CYHAL_DMA_PRIORITY_DEFAULT);
    pdm_pcm_assert_raise_val("PDM_PCM set DMA mode failed with return code %lx !", result);
}

// Read from PDM_PCM reg to provisioned DMA buffer
static void pdm_pcm_read_rxbuf(machine_pdm_pcm_obj_t *self) {
    // ToDo: Check the value of dma_half_buff_word_size and adapt for PDM-PCM
    uint16_t dma_half_buff_word_size = SIZEOF_DMA_BUFFER_IN_BYTES / 2;
    cy_rslt_t result = cyhal_pdm_pcm_read_async(&self->pdm_pcm_obj, self->dma_active_buf_p, dma_half_buff_word_size);
    pdm_pcm_assert_raise_val("PDM_PCM DMA read failed with return code %lx !", result);
}

// IRQ Handler
static void pdm_pcm_irq_handler(void *arg, cyhal_pdm_pcm_event_t event) {
    machine_pdm_pcm_obj_t *self = (machine_pdm_pcm_obj_t *)arg;

    if (0u != (event & CYHAL_PDM_PCM_ASYNC_COMPLETE)) {
        _dma_swap_active_dmabuf(self);
        pdm_pcm_read_rxbuf(self);
        _dma_copy_from_dmabuf_to_ringbuf(self);

        // ToDo: Implementation placeholder for non-blocking mode
        /*if ((self->io_mode == NON_BLOCKING) && (self->non_blocking_descriptor.copy_in_progress)) {
            fill_appbuf_from_ringbuf_non_blocking(self);
        }*/
    }
}

// Configure PDM_PCM IRQ
static void pdm_pcm_irq_configure(machine_pdm_pcm_obj_t *self) {
    cyhal_pdm_pcm_register_callback(&self->pdm_pcm_obj, &pdm_pcm_irq_handler, self);
    cyhal_pdm_pcm_enable_event(&self->pdm_pcm_obj, CYHAL_PDM_PCM_ASYNC_COMPLETE, CYHAL_ISR_PRIORITY_DEFAULT, true);
}

// Start PDM_PCM receive operation
static void pdm_pcm_start_rx(machine_pdm_pcm_obj_t *self) {
    pdm_pcm_read_rxbuf(self);
    cy_rslt_t result = cyhal_pdm_pcm_start(&self->pdm_pcm_obj);
    pdm_pcm_assert_raise_val("PDM_PCM start failed with return code %lx !", result);
}

// =======================================================================================
// MPY bindings for PDM_PCM (ports/psoc6)

// constructor()
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

// init.helper()
static void mp_machine_pdm_pcm_init_helper(machine_pdm_pcm_obj_t *self, mp_arg_val_t *args) {

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

    // !ToDo: Check the value of ibuf and if needs to be set by user
    int32_t ring_buffer_len = 20000; // !J: How can user calculate this? Easier if we set internally?
    if (ring_buffer_len < 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid ibuf"));
    }
    self->ibuf = ring_buffer_len;
    self->callback_for_non_blocking = MP_OBJ_NULL;
    self->io_mode = BLOCKING;
    self->ring_buffer_storage = m_new(uint8_t, ring_buffer_len);

    ringbuf_init(&self->ring_buffer, self->ring_buffer_storage, ring_buffer_len);
    pdm_pcm_audio_clock_init(audio_clock_freq_hz);
    pdm_pcm_init(self, &pdm_pcm_audio_clock);
    pdm_pcm_irq_configure(self);
    pdm_pcm_set_async_mode_dma(self);
    _dma_buff_init(self);
    pdm_pcm_start_rx(self);
}

// init()
static void mp_machine_pdm_pcm_init(machine_pdm_pcm_obj_t *self) {
    cyhal_pdm_pcm_start(&self->pdm_pcm_obj);
}

// deinit()
static void mp_machine_pdm_pcm_deinit(machine_pdm_pcm_obj_t *self) {
    cyhal_pdm_pcm_stop(&self->pdm_pcm_obj);
    cyhal_pdm_pcm_free(&self->pdm_pcm_obj);
    MP_STATE_PORT(machine_pdm_pcm_obj[self->pdm_pcm_id]) = NULL;
}

// =======================================================================================
// Implementation for stream protocol (ports/psoc6)

static mp_uint_t machine_pdm_pcm_stream_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    machine_pdm_pcm_obj_t *self = MP_OBJ_TO_PTR(self_in);

    uint8_t appbuf_sample_size_in_bytes = (self->bits / 8) * (self->format == STEREO ? 2: 1);
    // uint8_t appbuf_sample_size_in_bytes = (self->bits + 7) /8; // round up to higher limit. Consider 20 bits and 18 bits case.
    if (size % appbuf_sample_size_in_bytes != 0) { // size should be multiple of sample size
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }

    if (size == 0) {
        return 0;
    }

    if (self->io_mode == BLOCKING) { // blocking mode
        mp_buffer_info_t appbuf;
        appbuf.buf = (void *)buf_in; // To store read bits
        appbuf.len = size;
        #if MICROPY_PY_MACHINE_PDM_PCM_RING_BUF
        uint32_t num_bytes_read = fill_appbuf_from_ringbuf(self, &appbuf);
        #else
        uint32_t num_bytes_read = fill_appbuf_from_dma(self, &appbuf);
        #endif
        return num_bytes_read;
    } else { // // ToDo: Placeholder for non-blocking mode
        return 0;
    }
}

static const mp_stream_p_t pdm_pcm_stream_p = {
    .read = machine_pdm_pcm_stream_read,
    .is_text = false,
};

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
    protocol, &pdm_pcm_stream_p,
    locals_dict, &machine_pdm_pcm_locals_dict
    );

#endif // MICROPY_PY_MACHINE_PDM_PCM
