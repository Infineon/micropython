#ifndef MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H
#define MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H

#include <stdbool.h>

#include "py/obj.h"
#include "py/objlist.h"
#include "py/ringbuf.h"

// Port specific configuration.
#ifndef MICROPY_PY_BLUETOOTH_RINGBUF_SIZE
#define MICROPY_PY_BLUETOOTH_RINGBUF_SIZE (128)
#endif



#endif // MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H
