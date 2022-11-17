// Standard includes
#include <stdio.h>

// MPY interpreter related includes
#include "py/gc.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "shared/runtime/pyexec.h"

// MPY structure includes
#include "modmachine.h"

// MTB includes
#include "cybsp.h"
#include "cyhal.h"

#if MICROPY_PY_MACHINE

// Set machine classes here to be globally visible/accessible 
STATIC const mp_rom_map_elem_t machine_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_umachine) },
    { MP_ROM_QSTR(MP_QSTR_Pin),                 MP_ROM_PTR(&machine_pin_type) }
};
STATIC MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);


const mp_obj_module_t mp_module_machine = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&machine_module_globals,
};

// Register machine module in MPY
MP_REGISTER_MODULE(MP_QSTR_umachine, mp_module_machine);

#endif // MICROPY_PY_MACHINE
