#define MICROPY_PY_DEEPCRAFT_MPY (1)

#include "py/dynruntime.h"

#if !defined(__linux__)
void *memcpy(void *dst, const void *src, size_t n) {
    return mp_fun_table.memmove_(dst, src, n);
}
void *memset(void *s, int c, size_t n) {
    return mp_fun_table.memset_(s, c, n);
}
#endif

int native_errno=0;
#if defined(__linux__)
int *__errno_location (void)
#else
int *__errno (void)
#endif
{
    return &native_errno;
}

mp_obj_full_type_t dcmodel_type;

#include "examples/natmod/deepcraft/mp_src.c"

mp_map_elem_t dcmodel_locals_dict_table[3];
static MP_DEFINE_CONST_DICT(dcmodel_locals_dict, dcmodel_locals_dict_table);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    dcmodel_type.base.type = mp_fun_table.type_type;
    dcmodel_type.name = MP_QSTR_DEEPCRAFT;
    //MP_OBJ_TYPE_SET_SLOT(&dcmodel_type, make_new, &deflateio_make_new, 0);
    dcmodel_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_init), MP_OBJ_FROM_PTR(&init_obj) };
    dcmodel_locals_dict_table[1] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_enqueue), MP_OBJ_FROM_PTR(&enqueue_obj) };
    dcmodel_locals_dict_table[2] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_dequeue), MP_OBJ_FROM_PTR(&dequeue_obj) };
    MP_OBJ_TYPE_SET_SLOT(&dcmodel_type, locals_dict, (void*)&dcmodel_locals_dict, 2);

    mp_store_global(MP_QSTR___name__, MP_OBJ_NEW_QSTR(MP_QSTR_deepcraft));
    mp_store_global(MP_QSTR_DEEPCRAFT, MP_OBJ_FROM_PTR(&dcmodel_type));

    MP_DYNRUNTIME_INIT_EXIT
}
