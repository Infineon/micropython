// #if MICROPY_PY_MACHINE_PDM_PCM_RING_BUF
#include <stdlib.h>
#include <stdint.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"

typedef struct _ring_buf_t {
    uint8_t *buffer;
    size_t head;
    size_t tail;
    size_t size;
} ring_buf_t;

void ringbuf_init(ring_buf_t *rbuf, uint8_t datatype, size_t size);
// void ringbuf_init(ring_buf_t *rbuf, uint8_t *buffer, size_t size);
bool ringbuf_push(ring_buf_t *rbuf, uint8_t data);
bool ringbuf_pop(ring_buf_t *rbuf, uint8_t *data);
size_t ringbuf_available_data(ring_buf_t *rbuf);
size_t ringbuf_available_space(ring_buf_t *rbuf);

/*uint32_t fill_appbuf_from_ringbuf(ring_buf_t *rbuf, mp_buffer_info_t *appbuf);
void fill_appbuf_from_ringbuf_non_blocking(ring_buf_t *rbuf);*/

// static uint32_t fill_appbuf_from_ringbuf(machine_pdm_pcm_obj_t *self, mp_buffer_info_t *appbuf);
// static void fill_appbuf_from_ringbuf_non_blocking(machine_pdm_pcm_obj_t *self);

// #endif // MICROPY_PY_MACHINE_PDM_PCM_RING_BUF
