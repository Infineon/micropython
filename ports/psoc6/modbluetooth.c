// All includes may or may not be needed
#include "py/binary.h"
#include "py/gc.h"
#include "py/misc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "py/qstr.h"
#include "py/runtime.h"


#include "cycfg_bt_settings.h"
#include "modbluetooth.h"

#include "wiced_bt_stack.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>
#include "cybt_platform_trace.h"
#include "wiced_memory.h"
#include "cyhal.h"
#include "stdio.h"
#include "cycfg_gatt_db.h"
#include "cycfg_gap.h"
#include "wiced_bt_dev.h"
#include "cybsp_bt_config.h"

#include "extmod/modbluetooth.h"

// ******************************************************************************
#if MICROPY_PY_BLUETOOTH
wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data) {

    return WICED_BT_SUCCESS;

}

// extern const cybt_platform_config_t cybsp_bt_platform_cfg;
int mp_bluetooth_init(void) {
    // Initialize the Bluetooth hardware and stack.
    mp_printf(&mp_plat_print, "mp_bluetooth_init invoked \n");

    return 0; // Return 0 on success.

}

int mp_bluetooth_deinit() {
    return 0;
}

// Returns true when the Bluetooth stack is active.
bool mp_bluetooth_is_active(void) {
    return false;
}

// Gets the current address of this device in big-endian format.
void mp_bluetooth_get_current_address(uint8_t *addr_type, uint8_t *addr) {

}

// Sets the addressing mode to use.
void mp_bluetooth_set_address_mode(uint8_t addr_mode) {

}

// Get or set the GAP device name that will be used by service 0x1800, characteristic 0x2a00.
size_t mp_bluetooth_gap_get_device_name(const uint8_t **buf) {
    return 0;
}

int mp_bluetooth_gap_set_device_name(const uint8_t *buf, size_t len) {
    return 0;
}

// Start advertisement. Will re-start advertisement when already enabled.
// Returns errno on failure.
int mp_bluetooth_gap_advertise_start(bool connectable, int32_t interval_us, const uint8_t *adv_data, size_t adv_data_len, const uint8_t *sr_data, size_t sr_data_len) {
    return 0;
}

// Stop advertisement. No-op when already stopped.
void mp_bluetooth_gap_advertise_stop(void) {
    return;
}

// Start adding services. Must be called before mp_bluetooth_register_service.
int mp_bluetooth_gatts_register_service_begin(bool append) {
    return 0;
}
// Add a service with the given list of characteristics to the queue to be registered.
// The value_handles won't be valid until after mp_bluetooth_register_service_end is called.
int mp_bluetooth_gatts_register_service(mp_obj_bluetooth_uuid_t *service_uuid, mp_obj_bluetooth_uuid_t **characteristic_uuids, uint16_t *characteristic_flags, mp_obj_bluetooth_uuid_t **descriptor_uuids, uint16_t *descriptor_flags, uint8_t *num_descriptors, uint16_t *handles, size_t num_characteristics) {
    return 0;
}
// Register any queued services.
int mp_bluetooth_gatts_register_service_end(void) {
    return 0;
}

// Read the value from the local gatts db (likely this has been written by a central).
int mp_bluetooth_gatts_read(uint16_t value_handle, const uint8_t **value, size_t *value_len) {
    return 0;
}
// Write a value to the local gatts db (ready to be queried by a central). Optionally send notifications/indications.
int mp_bluetooth_gatts_write(uint16_t value_handle, const uint8_t *value, size_t value_len, bool send_update) {
    return 0;
}
// Send a notification/indication to the central, optionally with custom payload (otherwise the DB value is used).
int mp_bluetooth_gatts_notify_indicate(uint16_t conn_handle, uint16_t value_handle, int gatts_op, const uint8_t *value, size_t value_len) {
    return 0;
}

// Resize and enable/disable append-mode on a value.
// Append-mode means that remote writes will append and local reads will clear after reading.
int mp_bluetooth_gatts_set_buffer(uint16_t value_handle, size_t len, bool append) {
    return 0;
}

// Disconnect from a central or peripheral.
int mp_bluetooth_gap_disconnect(uint16_t conn_handle) {
    return 0;
}

// Set/get the MTU that we will respond to a MTU exchange with.
int mp_bluetooth_get_preferred_mtu(void) {
    return 0;
}

int mp_bluetooth_set_preferred_mtu(uint16_t mtu) {
    return 0;
}

#endif
