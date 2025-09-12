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
#include "stdlib.h"
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
#define ERRNO_BLUETOOTH_NOT_ACTIVE      MP_ENODEV
#define CASE_RETURN_STR(const)          case const: \
        return #const;

// Bring externs from MTB GeneratedSource/
extern uint8_t app_gap_device_name[];

// MPY defines and variables
volatile int mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_OFF;
static uint8_t address_mode = BLE_ADDR_PUBLIC;

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

    switch (event)
    {
        case BTM_ENABLED_EVT:
            /* Bluetooth Controller and Host Stack Enabled */
            if (WICED_BT_SUCCESS == p_event_data->enabled.status) {
                /* Perform application-specific initialization */
                ble_init();
            } else {
                mp_printf(&mp_plat_print, "Bluetooth disabled \n");
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
    bluetooth_assert_raise_val("Bluetooth Stack Initialization failed with error: ", wiced_result, WICED_BT_SUCCESS);
    if (WICED_BT_SUCCESS != wiced_result) {
        mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_STOPPING;
        return 0;
    }
    mp_bluetooth_ble_stack_state = MP_BLUETOOTH_BLE_STATE_ACTIVE;
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

void print_bd_address(wiced_bt_device_address_t bdadr) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X \n", bdadr[0], bdadr[1], bdadr[2], bdadr[3], bdadr[4], bdadr[5]);
}

// Gets the current address of this device in big-endian format.
void mp_bluetooth_get_current_address(uint8_t *addr_type, uint8_t *addr) {
    // wiced_bt_device_address_t bda = { 0 };
    if (!mp_bluetooth_is_active()) {
        mp_raise_OSError(ERRNO_BLUETOOTH_NOT_ACTIVE);
    }
    switch (address_mode) {
        case BLE_ADDR_PUBLIC:
            *addr_type = BLE_ADDR_PUBLIC;
            break;
        case BLE_ADDR_RANDOM:
            *addr_type = BLE_ADDR_RANDOM;
            break;
        default:
            mp_raise_OSError(MP_EINVAL);
    }

    wiced_bt_dev_read_local_addr(addr);
}

static void generate_random_address(wiced_bt_device_address_t addr) {
    cyhal_trng_t trng_obj;
    uint32_t random_data[2];
    cy_rslt_t rslt = cyhal_trng_init(&trng_obj);
    if (rslt != CY_RSLT_SUCCESS) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("TRNG init failed!\n"));
    }
    random_data[0] = cyhal_trng_generate(&trng_obj);
    random_data[1] = cyhal_trng_generate(&trng_obj);
    memcpy(addr, random_data, 6);
    cyhal_trng_free(&trng_obj);
}

// Sets the addressing mode to use.Alongwith it already sets the address as well based on mode selected
void mp_bluetooth_set_address_mode(uint8_t addr_mode) {
    wiced_bt_device_address_t addr;
    if (!mp_bluetooth_is_active()) {
        mp_raise_OSError(ERRNO_BLUETOOTH_NOT_ACTIVE);
    }
    switch (addr_mode) {
        case MP_BLUETOOTH_ADDRESS_MODE_PUBLIC: {
            address_mode = BLE_ADDR_PUBLIC;
            for (int i = 0; i < 5; i++) {
                addr[i] = *(cy_bt_device_address + i);
            }
            break;
        }
        case MP_BLUETOOTH_ADDRESS_MODE_RANDOM: {
            address_mode = BLE_ADDR_RANDOM;
            generate_random_address(addr);
            addr[5] = (addr[5] & 0x3F) | 0xC0; // MSB[0:1] - 11 - Static Random Address
            break;
        }
        // ToDo: This is in a way supported from MTB BLE stack side but is complicated to map to MPY side. Requires NVRAM to store the keys. Considering this is not an absolutely necessary feature, let's postpone for extension after MVP?
        case MP_BLUETOOTH_ADDRESS_MODE_RPA:
        case MP_BLUETOOTH_ADDRESS_MODE_NRPA:
            // Not yet supported.
            mp_raise_OSError(MP_EINVAL);
            break;
    }
    wiced_result_t wiced_result = wiced_bt_set_local_bdaddr(addr, address_mode);
    if (WICED_BT_SUCCESS != wiced_result) {
        mp_printf(&mp_plat_print, "Device address setting failed!\n");
    }
}

// Get or set the GAP device name that will be used by service 0x1800, characteristic 0x2a00.
size_t mp_bluetooth_gap_get_device_name(const uint8_t **buf) {
    *buf = (const uint8_t *)app_gap_device_name;
    return strlen((const char *)app_gap_device_name);
}

int mp_bluetooth_gap_set_device_name(const uint8_t *buf, size_t len) {
    mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("Feature unsupported: Device name cannot be set in run-time for this port.\n"));
    return MP_EOPNOTSUPP;
}

// Start advertisement. Will re-start advertisement when already enabled.
// Returns errno on failure.
int mp_bluetooth_gap_advertise_start(bool connectable, int32_t interval_us, const uint8_t *adv_data, size_t adv_data_len, const uint8_t *sr_data, size_t sr_data_len) {
    static uint8_t *temp_adv_data = NULL;
    static uint8_t *temp_sr_data = NULL;
    static uint32_t temp_adv_data_len = 0;
    static uint32_t temp_sr_data_len = 0;

    // Stop advertising if interval is 0
    if (interval_us == 0) {
        wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF, 0, NULL);
        return 0;
    }

    if (adv_data != NULL) {
        // Free previous data if it exists
        if (temp_adv_data != NULL) {
            free(temp_adv_data);
            temp_adv_data = NULL;
        }

        // Store new data (if not empty)
        if (adv_data_len > 0) {
            temp_adv_data = malloc(adv_data_len);
            if (temp_adv_data == NULL) {
                return -1; // Memory allocation failed
            }
            memcpy(temp_adv_data, adv_data, adv_data_len);
            temp_adv_data_len = adv_data_len;
        } else {
            temp_adv_data_len = 0;
        }
    }
    // Else: reuse previous temp_adv_data

    // Update scan response data if provided (not NULL)
    if (sr_data != NULL) {
        // Free previous data if it exists
        if (temp_sr_data != NULL) {
            free(temp_sr_data);
            temp_sr_data = NULL;
        }

        // Store new data (if not empty)
        if (sr_data_len > 0) {
            temp_sr_data = malloc(sr_data_len);
            if (temp_sr_data == NULL) {
                // Clean up advertising data if it was allocated in this call
                if (adv_data != NULL && temp_adv_data != NULL) {
                    free(temp_adv_data);
                    temp_adv_data = NULL;
                }
                return -1; // Memory allocation failed
            }
            memcpy(temp_sr_data, sr_data, sr_data_len);
            temp_sr_data_len = sr_data_len;
        } else {
            temp_sr_data_len = 0;
        }
    }
    // Else: reuse previous temp_sr_data

    // Set advertising data
    if (temp_adv_data_len > 0) {
        wiced_bt_ble_set_raw_advertisement_data(temp_adv_data_len, cy_bt_adv_packet_data);
    } else {
        wiced_bt_ble_set_raw_advertisement_data(0, NULL);
    }

    // Set scan response data
    if (temp_sr_data_len > 0) {
        wiced_bt_ble_set_raw_scan_response_data(temp_sr_data_len, cy_bt_adv_packet_data);
    } else {
        wiced_bt_ble_set_raw_scan_response_data(0, NULL);
    }

    // Determine advertising mode based on connectable parameter
    wiced_bt_ble_advert_mode_t advert_mode;
    if (connectable) {
        advert_mode = BTM_BLE_ADVERT_UNDIRECTED_HIGH; // Connectable undirected advertising[citation:1]
    } else {
        advert_mode = BTM_BLE_ADVERT_NONCONN_HIGH;    // Non-connectable advertising[citation:1]
    }


    // Start advertising
    wiced_result_t result = wiced_bt_start_advertisements(advert_mode, 0, NULL);

    return (result == WICED_BT_SUCCESS) ? 0 : -1;
}


// Stop advertisement. No-op when already stopped.
void mp_bluetooth_gap_advertise_stop(void) {
    wiced_result_t wiced_status;
    if (!mp_bluetooth_is_active()) {
        return;
    }
    wiced_status = wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF, 0, NULL);
    bluetooth_assert_raise_val("Stopping Bluetooth LE advertisements failed with error: ", wiced_status, WICED_SUCCESS);
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
// ToDo: wiced_bt_l2cap_le_get_peer_mtu gets the peer mtu but needs L2CAP connection established. Handle in L2CAP enablement // Default size is GATT_BLE_DEFAULT_MTU_SIZE : 23
int mp_bluetooth_get_preferred_mtu(void) {
    return GATT_BLE_DEFAULT_MTU_SIZE;
}

// ToDo: wiced_bt_gatt_client_configure_mtu allows configuring mtu size but needs GATT connection handle. Handle in GATT enablement.
int mp_bluetooth_set_preferred_mtu(uint16_t mtu) {
    if (!mp_bluetooth_is_active()) {
        return ERRNO_BLUETOOTH_NOT_ACTIVE;
    }
    return MP_EOPNOTSUPP;
}
#endif
