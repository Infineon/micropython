// Include the header file to get access to the MicroPython API
#include "py/dynruntime.h"
#include "model.h"
#include <string.h>

static mp_obj_t init(void){
    assign();
    IMAI_init();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(init_obj, init);

static mp_obj_t enqueue(const mp_obj_t data_in_obj){
    float data_in[IMAI_DATA_IN_COUNT];
    mp_obj_t *data_in_items;
    size_t len;
    mp_obj_get_array(data_in_obj, &len, &data_in_items);
    if (len != IMAI_DATA_IN_COUNT) {
        mp_raise_ValueError("data_in must be a list of IMAI_DATA_IN_COUNT floats");
    }
    for (int i = 0; i < IMAI_DATA_IN_COUNT; i++) {
        data_in[i] = mp_obj_get_float(data_in_items[i]);
    }
    int result = IMAI_enqueue(data_in);
    return MP_OBJ_NEW_SMALL_INT(result);
}
MP_DEFINE_CONST_FUN_OBJ_1(enqueue_obj, enqueue);

static mp_obj_t dequeue(mp_obj_t data_out_obj) {
    mp_buffer_info_t buf_info;
    mp_get_buffer(data_out_obj, &buf_info, MP_BUFFER_WRITE);
    float *data_out = (float *)buf_info.buf;
    int result = IMAI_dequeue(data_out);
    if (result == 0) {
        return MP_OBJ_NEW_SMALL_INT(result);
    } else if (result == -1) {
        return MP_OBJ_NEW_SMALL_INT(result);
    } else if (result == -2) {
        mp_raise_ValueError(MP_ERROR_TEXT("Internal memory allocation error"));
    }
    return MP_OBJ_NEW_SMALL_INT(result);
}
MP_DEFINE_CONST_FUN_OBJ_1(dequeue_obj, dequeue);