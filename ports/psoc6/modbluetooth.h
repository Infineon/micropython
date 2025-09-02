#ifndef MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H
#define MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H

#include <stdbool.h>

#include "py/obj.h"
#include "py/objlist.h"
#include "py/ringbuf.h"

enum {
    MP_BLUETOOTH_BLE_STATE_OFF,
    MP_BLUETOOTH_BLE_STATE_STARTING,
    MP_BLUETOOTH_BLE_STATE_WAITING_FOR_SYNC,
    MP_BLUETOOTH_BLE_STATE_ACTIVE,
    MP_BLUETOOTH_BLE_STATE_STOPPING,
};

extern volatile int mp_bluetooth_ble_stack_state;

#define bluetooth_assert_raise_val(msg, ret)   if (ret != CY_RSLT_SUCCESS) { \
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT(msg), ret); \
}

// Port specific configuration.
#ifndef MICROPY_PY_BLUETOOTH_RINGBUF_SIZE
#define MICROPY_PY_BLUETOOTH_RINGBUF_SIZE (128)
#endif



#endif // MICROPY_INCLUDED_PSOC6_MODBLUETOOTH_H
