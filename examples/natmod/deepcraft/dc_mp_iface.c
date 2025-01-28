/* This example demonstrates the following features in a native module:
    - defining simple functions exposed to Python
    - defining local, helper C functions
    - defining constant integers and strings exposed to Python
    - getting and creating integer objects
    - creating Python lists
    - raising exceptions
    - allocating memory
    - BSS and constant data (rodata)
    - relocated pointers in rodata
*/

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


// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    // This must be first, it sets up the globals dict and other things
    MP_DYNRUNTIME_INIT_ENTRY

    // Messages can be printed as usual
    mp_printf(&mp_plat_print, "initialising deepcraft module=%p\n", self);

    // Make the functions available in the module's namespace
    mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&init_obj));
    mp_store_global(MP_QSTR_enqueue, MP_OBJ_FROM_PTR(&enqueue_obj));
    mp_store_global(MP_QSTR_dequeue, MP_OBJ_FROM_PTR(&dequeue_obj));

    // Add some constants to the module's namespace
    mp_store_global(MP_QSTR_DATA_IN, MP_OBJ_NEW_SMALL_INT(IMAI_DATA_IN_COUNT));
    mp_store_global(IMAI_DATA_OUT_COUNT, MP_OBJ_NEW_SMALL_INT(IMAI_DATA_OUT_COUNT));
    mp_store_global(MP_QSTR_MSG, MP_OBJ_NEW_QSTR(MP_QSTR_HELLO_MICROPYTHON));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
