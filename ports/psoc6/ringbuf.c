#include "ringbuf.h"

#if MICROPY_PY_MACHINE_PDM_PCM_RING_BUF

void ringbuf_init(ring_buf_t *rbuf, uint8_t datatype, size_t size) {
    /*if(datatype == 1){
        rbuf->buffer = (uint8_t *)m_new(uint8_t, size);
    } else if(datatype == 2){
        rbuf->buffer = (uint16_t *)m_new(uint16_t, size);
    } else if(datatype == 4){
        rbuf->buffer = (uint32_t *)m_new(uint32_t, size);
    }*/
    rbuf->buffer = m_new(uint8_t, size);
    // rbuf->buffer = buffer;
    rbuf->size = size;
    rbuf->head = 0;
    rbuf->tail = 0;
}

bool ringbuf_push(ring_buf_t *rbuf, uint8_t data) {
    size_t next_tail = (rbuf->tail + 1) % rbuf->size;

    if (next_tail != rbuf->head) {
        rbuf->buffer[rbuf->tail] = data;
        rbuf->tail = next_tail;
        return true;
    }
    // full
    return false;
}

bool ringbuf_pop(ring_buf_t *rbuf, uint8_t *data) {
    if (rbuf->head == rbuf->tail) {
        // empty
        return false;
    }
    *data = rbuf->buffer[rbuf->head];
    rbuf->head = (rbuf->head + 1) % rbuf->size;
    return true;
}

size_t ringbuf_available_data(ring_buf_t *rbuf) {
    return (rbuf->tail - rbuf->head + rbuf->size) % rbuf->size;
}

size_t ringbuf_available_space(ring_buf_t *rbuf) {
    return rbuf->size - ringbuf_available_data(rbuf) - 1;
}

/*uint32_t fill_appbuf_from_ringbuf(ring_buf_t *rbuf, mp_buffer_info_t *appbuf) {
    uint32_t num_bytes_copied_to_appbuf = 0;
    uint32_t *app_p = (uint32_t *)appbuf->buf;

    uint32_t num_bytes_needed_from_ringbuf = appbuf->len;

    while (num_bytes_needed_from_ringbuf) {
        //printf("num_bytes_needed_from_ringbuf: %ld \r\n", num_bytes_needed_from_ringbuf);
        // poll the ringbuf until a sample becomes available,  copy into appbuf using the mapping transform
        while (ringbuf_pop(rbuf, app_p) == false) {
                        ;
        }
        //cyhal_gpio_write(CYBSP_USER_LED, 0);
        num_bytes_copied_to_appbuf++;
        num_bytes_needed_from_ringbuf--;
        app_p += num_bytes_copied_to_appbuf;
        //printf("num_bytes_copied_to_appbuf: %ld\r\n", num_bytes_copied_to_appbuf);
    }
    return num_bytes_copied_to_appbuf;
}*/

// Copy from ringbuf to appbuf as soon as ASYNC_TRANSFER_COMPLETE is triggered
void fill_appbuf_from_ringbuf_non_blocking(ring_buf_t *rbuf) {
    /*uint32_t num_bytes_copied_to_appbuf = 0;
    uint8_t *app_p = &(((uint8_t *)self->non_blocking_descriptor.appbuf.buf)[self->non_blocking_descriptor.index]);

    uint8_t appbuf_sample_size_in_bytes = (self->bits == 16? 2 : 4) * (self->format == STEREO ? 2: 1);
    uint32_t num_bytes_remaining_to_copy_to_appbuf = self->non_blocking_descriptor.appbuf.len - self->non_blocking_descriptor.index;
    uint32_t num_bytes_remaining_to_copy_from_ring_buffer = num_bytes_remaining_to_copy_to_appbuf *
        (PDM_PCM_RX_FRAME_SIZE_IN_BYTES / appbuf_sample_size_in_bytes);
    uint32_t num_bytes_needed_from_ringbuf = MIN(SIZEOF_NON_BLOCKING_COPY_IN_BYTES, num_bytes_remaining_to_copy_from_ring_buffer);
    uint8_t discard_byte;
    if (ringbuf_available_data(&self->ring_buffer) >= num_bytes_needed_from_ringbuf) {
        while (num_bytes_needed_from_ringbuf) {

            uint8_t f_index = get_frame_mapping_index(self->bits, self->format);

            for (uint8_t i = 0; i < PDM_PCM_RX_FRAME_SIZE_IN_BYTES; i++) {
                int8_t r_to_a_mapping = pdm_pcm_frame_map[f_index][i];
                if (r_to_a_mapping != -1) {
                    ringbuf_pop(&self->ring_buffer, app_p + r_to_a_mapping);
                    num_bytes_copied_to_appbuf++;
                } else { // r_a_mapping == -1
                    // discard unused byte from ring buffer
                    ringbuf_pop(&self->ring_buffer, &discard_byte);
                }
                num_bytes_needed_from_ringbuf--;
            }
            app_p += appbuf_sample_size_in_bytes;
        }
        self->non_blocking_descriptor.index += num_bytes_copied_to_appbuf;

        if (self->non_blocking_descriptor.index >= self->non_blocking_descriptor.appbuf.len) {
            self->non_blocking_descriptor.copy_in_progress = false;
            mp_sched_schedule(self->callback_for_non_blocking, MP_OBJ_FROM_PTR(self));
        }
    }*/
}



#endif // MICROPY_PY_MACHINE_PDM_PCM_RING_BUF
