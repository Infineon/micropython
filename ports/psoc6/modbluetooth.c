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
#include "modbluetooth.h"

#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"

// ******************************************************************************
#if MICROPY_PY_BLUETOOTH
#define NUM_ADV_PACKETS                 (3u)

#define CASE_RETURN_STR(const)          case const: \
        return #const;

volatile int mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_OFF;

// ToDo: Add appropriate BLE device name in GeneratedSource/cycfg_bt_settings.c. Currently it is set to "Hello"

// *********************************** Utility API's *******************************************

// ToDo: Utility API's can go to another file and if possible generalized? Could reuse same for logging errors better?

// Converts the wiced_bt_gatt_status_t enum value to its corresponding string literal
const char *get_bt_gatt_status_name(wiced_bt_gatt_status_t status) {
    switch ((int)status)
    {
        CASE_RETURN_STR(WICED_BT_GATT_SUCCESS)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_HANDLE)
        CASE_RETURN_STR(WICED_BT_GATT_READ_NOT_PERMIT)
        CASE_RETURN_STR(WICED_BT_GATT_WRITE_NOT_PERMIT)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_PDU)
        CASE_RETURN_STR(WICED_BT_GATT_INSUF_AUTHENTICATION)
        CASE_RETURN_STR(WICED_BT_GATT_REQ_NOT_SUPPORTED)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_OFFSET)
        CASE_RETURN_STR(WICED_BT_GATT_INSUF_AUTHORIZATION)
        CASE_RETURN_STR(WICED_BT_GATT_PREPARE_Q_FULL)
        CASE_RETURN_STR(WICED_BT_GATT_ATTRIBUTE_NOT_FOUND)
        CASE_RETURN_STR(WICED_BT_GATT_NOT_LONG)
        CASE_RETURN_STR(WICED_BT_GATT_INSUF_KEY_SIZE)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_ATTR_LEN)
        CASE_RETURN_STR(WICED_BT_GATT_ERR_UNLIKELY)
        CASE_RETURN_STR(WICED_BT_GATT_INSUF_ENCRYPTION)
        CASE_RETURN_STR(WICED_BT_GATT_UNSUPPORT_GRP_TYPE)
        CASE_RETURN_STR(WICED_BT_GATT_INSUF_RESOURCE)
        CASE_RETURN_STR(WICED_BT_GATT_DATABASE_OUT_OF_SYNC)
        CASE_RETURN_STR(WICED_BT_GATT_VALUE_NOT_ALLOWED)
        CASE_RETURN_STR(WICED_BT_GATT_ILLEGAL_PARAMETER)
        CASE_RETURN_STR(WICED_BT_GATT_NO_RESOURCES)
        CASE_RETURN_STR(WICED_BT_GATT_INTERNAL_ERROR)
        CASE_RETURN_STR(WICED_BT_GATT_WRONG_STATE)
        CASE_RETURN_STR(WICED_BT_GATT_DB_FULL)
        CASE_RETURN_STR(WICED_BT_GATT_BUSY)
        CASE_RETURN_STR(WICED_BT_GATT_ERROR)
        CASE_RETURN_STR(WICED_BT_GATT_CMD_STARTED)
        CASE_RETURN_STR(WICED_BT_GATT_PENDING)
        CASE_RETURN_STR(WICED_BT_GATT_AUTH_FAIL)
        CASE_RETURN_STR(WICED_BT_GATT_MORE)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_CFG)
        CASE_RETURN_STR(WICED_BT_GATT_SERVICE_STARTED)
        CASE_RETURN_STR(WICED_BT_GATT_ENCRYPTED_NO_MITM)
        CASE_RETURN_STR(WICED_BT_GATT_NOT_ENCRYPTED)
        CASE_RETURN_STR(WICED_BT_GATT_CONGESTED)
        CASE_RETURN_STR(WICED_BT_GATT_NOT_ALLOWED)
        CASE_RETURN_STR(WICED_BT_GATT_HANDLED)
        CASE_RETURN_STR(WICED_BT_GATT_NO_PENDING_OPERATION)
        CASE_RETURN_STR(WICED_BT_GATT_INDICATION_RESPONSE_PENDING)
        CASE_RETURN_STR(WICED_BT_GATT_WRITE_REQ_REJECTED)
        CASE_RETURN_STR(WICED_BT_GATT_CCC_CFG_ERR)
        CASE_RETURN_STR(WICED_BT_GATT_PRC_IN_PROGRESS)
        CASE_RETURN_STR(WICED_BT_GATT_OUT_OF_RANGE)
        CASE_RETURN_STR(WICED_BT_GATT_BAD_OPCODE)
        CASE_RETURN_STR(WICED_BT_GATT_INVALID_CONNECTION_ID)
    }
    return "UNKNOWN_STATUS";
}


// *********************************** MTB wrapper API's and callbacks *******************************************

// ToDo : Add better function names to clearly differentiate API's purely using MTB vs MPY vs MPY+MTB
static void start_advertisement(void) {
    wiced_result_t wiced_status;

    /* Set Advertisement Data */
    wiced_status = wiced_bt_ble_set_raw_advertisement_data(NUM_ADV_PACKETS,
        cy_bt_adv_packet_data);
    if (WICED_SUCCESS != wiced_status) {
        mp_printf(&mp_plat_print, "Raw advertisement failed with err 0x%x\n", wiced_status);
    }

    /* Do not allow peer to pair */
    wiced_bt_set_pairable_mode(WICED_TRUE, FALSE);

    /* Start Undirected LE Advertisements on device startup. */
    wiced_status = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
        BLE_ADDR_PUBLIC,
        NULL);

    // mp_printf(&mp_plat_print, "Start adv status: %d\r\n", wiced_status);

    if (WICED_SUCCESS != wiced_status) {
        mp_printf(&mp_plat_print, "Starting undirected Bluetooth LE advertisements"
            "Failed with err 0x%x\n", wiced_status);
    }

}

// Handles GATT events from the BT stack
static wiced_bt_gatt_status_t gatt_event_callback(wiced_bt_gatt_evt_t event,
    wiced_bt_gatt_event_data_t *p_event_data) {
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    switch (event)
    {
        case GATT_CONNECTION_STATUS_EVT:
            if (p_event_data->connection_status.connected) {
                mp_printf(&mp_plat_print, "Device connected\n");
            } else {
                mp_printf(&mp_plat_print, "Device disconnected\n");
                /* Restart advertising when disconnected */
                start_advertisement();
            }
            break;

        default:
            break;
    }

    return gatt_status;
}

// Initialize mandatory sub-layers
static void ble_init() {
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_ERROR;

    /* Register with BT stack to receive GATT callback */
    gatt_status = wiced_bt_gatt_register(gatt_event_callback);

    /* Initialize GATT Database */
    gatt_status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);
    if (WICED_BT_GATT_SUCCESS != gatt_status) {
        mp_printf(&mp_plat_print, "\n GATT DB Initialization failed with err 0x%x\n", gatt_status);
    }

    /* Start Undirected LE Advertisements on device startup. */
    start_advertisement();
}

// Receive management events from the LE stack
wiced_result_t bt_management_callback(wiced_bt_management_evt_t event,
    wiced_bt_management_evt_data_t *p_event_data) {
    wiced_result_t wiced_result = WICED_BT_SUCCESS;
    wiced_bt_device_address_t bda = { 0 };

    switch (event)
    {
        case BTM_ENABLED_EVT:
            /* Bluetooth Controller and Host Stack Enabled */
            if (WICED_BT_SUCCESS == p_event_data->enabled.status) {
                wiced_result = wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_PUBLIC);
                wiced_bt_dev_read_local_addr(bda);

                /* Perform application-specific initialization */
                ble_init();
            } else {
                mp_printf(&mp_plat_print, "Bluetooth Disabled \n");
            }
            break;

        case BTM_DISABLED_EVT:
            mp_printf(&mp_plat_print, "Bluetooth disabled\n");
            break;

        case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
            /* Set IO capabilities for pairing */
            p_event_data->pairing_io_capabilities_ble_request.local_io_cap = BTM_IO_CAPABILITIES_NONE;
            p_event_data->pairing_io_capabilities_ble_request.oob_data = FALSE;
            p_event_data->pairing_io_capabilities_ble_request.auth_req = BTM_LE_AUTH_REQ_SC_MITM_BOND;
            break;

        case BTM_PAIRING_COMPLETE_EVT:
            if (p_event_data->pairing_complete.pairing_complete_info.ble.reason == WICED_BT_SUCCESS) {
                mp_printf(&mp_plat_print, "Pairing successful with device: ");
                for (int i = 0; i < 6; i++)
                {
                    mp_printf(&mp_plat_print, "%02X:", p_event_data->pairing_complete.bd_addr[5 - i]);
                }
                printf("\n");
            } else {
                mp_printf(&mp_plat_print, "Pairing failed, reason: 0x%02X\n", p_event_data->pairing_complete.pairing_complete_info.ble.reason);
            }
            break;

        case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
            if (p_event_data->ble_advert_state_changed == BTM_BLE_ADVERT_OFF) {
                mp_printf(&mp_plat_print, "Advertising stopped\n");
            }
            break;

        default:
            break;
    }

    return wiced_result;
}


// ============================================================================
int mp_bluetooth_init(void) {
    wiced_result_t wiced_result;
    // Initialize the Bluetooth hardware and stack.

    if (mp_bluetooth_ble_stack_state == MP_BLUETOOTH_BLE_STATE_ACTIVE) {
        return 0;
    }

    mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_STARTING;

    /* Configure platform specific settings for the BT device */
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* Register call back and configuration with stack */
    wiced_result = wiced_bt_stack_init(bt_management_callback, &wiced_bt_cfg_settings);
    if (WICED_BT_SUCCESS == wiced_result) {
        mp_printf(&mp_plat_print, "Bluetooth Stack Initialization Successful \n");
        mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_ACTIVE;
    } else {
        mp_printf(&mp_plat_print, "Bluetooth Stack Initialization failed \n");
        mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_STOPPING;
    }

    return 0;
}

int mp_bluetooth_deinit() {
    if (mp_bluetooth_ble_stack_state == MP_BLUETOOTH_BLE_STATE_OFF) {
        return 0;
    }
    // wiced_bt_gatt_disconnect(conn_id) //ToDo in GATT API's
    wiced_bt_stack_deinit();
    mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_OFF;
    return 0;
}

// Returns true when the Bluetooth stack is active.
bool mp_bluetooth_is_active(void) {
    return mp_bluetooth_ble_stack_state == MP_BLUETOOTH_BLE_STATE_ACTIVE;
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
