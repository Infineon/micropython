
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

#include "cycfg_bt_settings.h"
#include "cybsp_bt_config.h"
#include "wiced_bt_stack.h"



// ******************************************************************************
#if MICROPY_PY_BLUETOOTH
#define NUM_ADV_PACKETS                 (3u)
#define ERRNO_BLUETOOTH_NOT_ACTIVE      MP_ENODEV
#define CASE_RETURN_STR(const)          case const: \
        return #const;


// Convert microseconds to BLE slots (1 slot = 0.625 ms)
#define US_TO_SLOTS(us) ((uint16_t)((us) / 625))

// Bring externs from MTB GeneratedSource/
extern uint8_t app_gap_device_name[];

wiced_bt_ble_advert_elem_t cy_bt_adv_pkt[8];    // Max 8 elements possible

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

void free_advert_elements(wiced_bt_ble_advert_elem_t *adv_elements, uint8_t num_elements) {
    if (adv_elements) {
        for (uint8_t i = 0; i < num_elements; i++) {
            if (adv_elements[i].p_data) {
                free(adv_elements[i].p_data);
            }
        }
        free(adv_elements);
    }
}

wiced_result_t parse_ltv_to_advert_elements(const uint8_t *adv_data, uint16_t adv_len,
    wiced_bt_ble_advert_elem_t **adv_elements,
    uint8_t *num_elements) {

    if (!adv_data || adv_len == 0 || !adv_elements || !num_elements) {
        return WICED_BT_ERROR;
    }

    // Count the number of elements first
    uint8_t element_count = 0;
    uint16_t offset = 0;
    while (offset < adv_len) {
        if (offset + 1 > adv_len) {
            break;                       // Insufficient data

        }
        uint8_t field_len = adv_data[offset];
        if (field_len == 0 || offset + field_len + 1 > adv_len) {
            break;
        }

        element_count++;
        offset += field_len + 1;
    }

    if (element_count == 0) {
        return WICED_BT_ERROR;
    }

    // Allocate memory for elements
    *adv_elements = (wiced_bt_ble_advert_elem_t *)malloc(element_count * sizeof(wiced_bt_ble_advert_elem_t));
    if (!*adv_elements) {
        return WICED_BT_NO_RESOURCES;
    }

    // Parse and populate elements
    offset = 0;
    *num_elements = 0;

    while (offset < adv_len && *num_elements < element_count) {
        uint8_t field_len = adv_data[offset];
        uint8_t field_type = adv_data[offset + 1];

        if (field_len == 0 || offset + field_len + 1 > adv_len) {
            break;
        }

        // Allocate memory for this element's data
        uint8_t *element_data = (uint8_t *)malloc(field_len);
        if (!element_data) {
            // Cleanup on failure
            for (int i = 0; i < *num_elements; i++) {
                free((*adv_elements)[i].p_data);
            }
            free(*adv_elements);
            *adv_elements = NULL;
            return WICED_BT_NO_RESOURCES;
        }

        // Copy the data (excluding the length byte)
        memcpy(element_data, &adv_data[offset + 1], field_len);

        // Map Bluetooth SIG AD types to WICED BLE advert types
        wiced_bt_ble_advert_elem_t *element = &(*adv_elements)[*num_elements];
        element->len = field_len;
        element->p_data = element_data;

        // Map common AD types
        switch (field_type) {
            case 0x01: // Flags
                element->advert_type = BTM_BLE_ADVERT_TYPE_FLAG;
                break;
            case 0x02: // Incomplete List of 16-bit Service Class UUIDs
                element->advert_type = BTM_BLE_ADVERT_TYPE_16SRV_PARTIAL;
                break;
            case 0x03: // Complete List of 16-bit Service Class UUIDs
                element->advert_type = BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE;
                break;
            case 0x06: // Incomplete List of 128-bit Service Class UUIDs
                element->advert_type = BTM_BLE_ADVERT_TYPE_128SRV_PARTIAL;
                break;
            case 0x07: // Complete List of 128-bit Service Class UUIDs
                element->advert_type = BTM_BLE_ADVERT_TYPE_128SRV_COMPLETE;
                break;
            case 0x08: // Shortened Local Name
                element->advert_type = BTM_BLE_ADVERT_TYPE_NAME_SHORT;
                break;
            case 0x09: // Complete Local Name
                element->advert_type = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
                break;
            case 0x0A: // Tx Power Level
                element->advert_type = BTM_BLE_ADVERT_TYPE_TX_POWER;
                break;
            case 0x16: // Service Data - 16-bit UUID
                element->advert_type = BTM_BLE_ADVERT_TYPE_SERVICE_DATA;
                break;
            case 0xFF: // Manufacturer Specific Data
                element->advert_type = BTM_BLE_ADVERT_TYPE_MANUFACTURER;
                break;
            default:
                // For unknown types, use raw data type
                element->advert_type = BTM_BLE_ADVERT_TYPE_MANUFACTURER;
                break;
        }

        (*num_elements)++;
        offset += field_len + 1;
    }

    return WICED_SUCCESS;
}

// *********************************** MTB wrapper API's and callbacks *******************************************

// ToDo : Add better function names to clearly differentiate API's purely using MTB vs MPY vs MPY+MTB

static void stop_advertisement(void) {
    wiced_result_t status = WICED_BT_SUCCESS;
    status = wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF, 0, NULL);
    bluetooth_assert_raise_val("Stopping Bluetooth LE advertisements failed with error: ", status, WICED_SUCCESS);
}


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

        case BTM_BLE_SCAN_STATE_CHANGED_EVT:

            if (p_event_data->ble_scan_state_changed == BTM_BLE_SCAN_TYPE_HIGH_DUTY) {
                printf("Scan State Change: BTM_BLE_SCAN_TYPE_HIGH_DUTY\n");
            } else if (p_event_data->ble_scan_state_changed == BTM_BLE_SCAN_TYPE_LOW_DUTY) {
                printf("Scan State Change: BTM_BLE_SCAN_TYPE_LOW_DUTY\n");
            } else if (p_event_data->ble_scan_state_changed == BTM_BLE_SCAN_TYPE_NONE) {
                printf("Scan stopped\n");
            } else {
                printf("Invalid scan state\n");
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

int mp_bluetooth_gap_advertise_start(bool connectable, int32_t interval_us,
    const uint8_t *adv_data, size_t adv_data_len,
    const uint8_t *sr_data, size_t sr_data_len) {

    if (!mp_bluetooth_is_active()) {
        return ERRNO_BLUETOOTH_NOT_ACTIVE;
    }

    wiced_result_t result = WICED_BT_SUCCESS;
    // Static storage for last used adv/scanned data
    static wiced_bt_ble_advert_elem_t *last_adv_elements = NULL;
    static uint8_t last_num_adv_elements = 0;
    static wiced_bt_ble_advert_elem_t *last_sr_elements = NULL;
    static uint8_t last_num_sr_elements = 0;

    // Stop advertising if interval_us is 0 or None
    if (interval_us == 0) {
        stop_advertisement();
        return 0;
    } else {
        mp_printf(&mp_plat_print, "Advertising interval is ignored and set to default interval internally - 0x0020 to 0x4000\r\n");
    }

    // Handle adv_data
    if (adv_data == NULL) {
        // Reuse previous
        adv_data = NULL;
        adv_data_len = 0;
    }
    if (adv_data_len == 0 && last_adv_elements) {
        // Clear previous
        free_advert_elements(last_adv_elements, last_num_adv_elements);
        last_adv_elements = NULL;
        last_num_adv_elements = 0;
    }
    if (adv_data && adv_data_len > 0) {
        if (last_adv_elements) {
            free_advert_elements(last_adv_elements, last_num_adv_elements);
        }
        result = parse_ltv_to_advert_elements(adv_data, adv_data_len, &last_adv_elements, &last_num_adv_elements);
        if (result != WICED_SUCCESS) {
            return result;
        }
    }

    // Handle scan response data
    if (sr_data == NULL) {
        sr_data = NULL;
        sr_data_len = 0;
    }
    if (sr_data_len == 0 && last_sr_elements) {
        free_advert_elements(last_sr_elements, last_num_sr_elements);
        last_sr_elements = NULL;
        last_num_sr_elements = 0;
    }
    if (sr_data && sr_data_len > 0) {
        if (last_sr_elements) {
            free_advert_elements(last_sr_elements, last_num_sr_elements);
        }
        result = parse_ltv_to_advert_elements(sr_data, sr_data_len, &last_sr_elements, &last_num_sr_elements);
        if (result != WICED_SUCCESS) {
            return result;
        }
    }

    // Set advertisement data
    result = wiced_bt_ble_set_raw_advertisement_data(last_num_adv_elements, last_adv_elements);
    if (result != WICED_SUCCESS) {
        return result;
    }

    if (sr_data && sr_data_len > 0) {
        result = wiced_bt_ble_set_raw_scan_response_data(last_num_sr_elements, last_sr_elements);
        if (result != WICED_SUCCESS) {
            return result;
        }
    }

    // Start advertising
    result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, BLE_ADDR_PUBLIC, NULL);

    return result;
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

#if MICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE

static void isr_timer(void *callback_arg, cyhal_timer_event_t event) {
    // printf("Tim int\r\n");
    mp_bluetooth_gap_scan_stop();
}

static cyhal_timer_t mp_bluetooth_gap_scan_timer_obj;

void mp_bluetooth_gap_scan_timer_start(int32_t duration_ms) {
    uint32_t period_hal;        // Period/count input for the PSoC6 HAL timer configuration
    uint32_t fz_hal = 1000000;  // Frequency for the PSoC timer clock is fixed as 1 MHz
    period_hal = (uint32_t)(duration_ms * fz_hal) - 1; // Overflow Period = (Period + 1)/ frequency ;period = (overflow period * frequency)-1

    // Adjust the frequency & recalculate the period if period/count is  greater than the maximum overflow value for a 32 bit timer ie; 2^32
    while (period_hal > 4294967296) {
        fz_hal = fz_hal / 10;  // Reduce the fz_hal value by 10%
        period_hal = (uint32_t)(duration_ms * fz_hal) - 1;  // Recalculate Period input for the PSoC6 HAL timer configuration
    }

    // Timer initialisation of port
    cy_rslt_t rslt;
    period_hal = 13000; // For testing purpose only, remove later//9999

    const cyhal_timer_cfg_t timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = period_hal,               /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = 0,                 /* Run the timer */
        .value = 0                          /* Initial value of counter */
    };

    /* Initialize the timer object. Does not use pin output ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */

    rslt = cyhal_timer_init(&mp_bluetooth_gap_scan_timer_obj, NC, NULL);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);

    /* Apply timer configuration such as period, count direction, run mode, etc. */
    rslt = cyhal_timer_configure(&mp_bluetooth_gap_scan_timer_obj, &timer_cfg);

    /* Set the frequency of timer to Defined frequency */
    rslt = cyhal_timer_set_frequency(&mp_bluetooth_gap_scan_timer_obj, fz_hal);

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&mp_bluetooth_gap_scan_timer_obj, isr_timer, NULL);

    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&mp_bluetooth_gap_scan_timer_obj, CYHAL_TIMER_IRQ_TERMINAL_COUNT, 3, true);

    /* Start the timer with the configured settings */
    rslt = cyhal_timer_start(&mp_bluetooth_gap_scan_timer_obj);

    CY_ASSERT(CY_RSLT_SUCCESS == rslt);

}

void print_bd_address(wiced_bt_device_address_t bdadr) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X \n", bdadr[0], bdadr[1], bdadr[2], bdadr[3], bdadr[4], bdadr[5]);
}

void ctss_scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result,
    uint8_t *p_adv_data) {
    printf("Yayy\r\n");
}

// void ctss_scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result,
//                            uint8_t *p_adv_data )
// {
//    wiced_result_t         result = WICED_BT_SUCCESS;
//    uint8_t                length = 0u;
//    uint8_t                *adv_name;
//    uint8_t                client_device_name[15] = {'C','T','S',' ','C','l',
//                                                     'i','e','n','t','\0'};
//
//    if (p_scan_result)
//    {
//        mp_bluetooth_gap_on_scan_result(p_scan_result->ble_addr_type, p_scan_result->remote_bd_addr, p_scan_result->flag, p_scan_result->rssi, p_adv_data, sizeof(p_adv_data));
//        adv_name = wiced_bt_ble_check_advertising_data(p_adv_data,
//                                BTM_BLE_ADVERT_TYPE_NAME_COMPLETE,
//                                                         &length);
//        if(NULL == adv_name)
//        {
//            return;
//        }
//        /* Check if the peer device's name is "BLE CTS Client" */
//        if(0 == memcmp(adv_name, client_device_name, strlen((const char *)client_device_name)))
//        {
//            printf("\nFound the peer device! BD Addr: ");
//            print_bd_address(p_scan_result->remote_bd_addr);
//
//            /* Device found. Stop scan. */
//            if((result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE,
//                                           ctss_scan_result_cback))!= WICED_BT_SUCCESS)
//            {
//                printf("\r\nscan off status %d\n", result);
//            }
//            else
//            {
//                printf("Scan completed\n\n");
//            }
//
//            printf("Initiating connection\n");
//            /* Initiate the connection */
//            if(wiced_bt_gatt_le_connect(p_scan_result->remote_bd_addr,
//                                        p_scan_result->ble_addr_type,
//                                        BLE_CONN_MODE_HIGH_DUTY,
//                                        WICED_TRUE)!= WICED_TRUE)
//            {
//                printf("\rwiced_bt_gatt_connect failed\n");
//            }
//        }
//        else
//        {
//            printf("BD Addr: ");
//            print_bd_address(p_scan_result->remote_bd_addr);
//            return;    //Skip - This is not the device we are looking for.
//        }
//    }
// }

int mp_bluetooth_gap_scan_start(int32_t duration_ms, int32_t interval_us, int32_t window_us, bool active_scan) {
    // Stop any ongoing GAP scan.
    wiced_result_t result;
    /*int ret = mp_bluetooth_gap_scan_stop();
    if (ret) {
        return ret;
    }*/
    mp_bluetooth_gap_scan_timer_start(duration_ms);

    for (;;) {
        result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, ctss_scan_result_cback);
        if ((WICED_BT_PENDING == result) || (WICED_BT_BUSY == result)) {
            printf("Scanning...\r\n");
            // vTaskDelay(10);
            // cyhal_system_delay_ms(1);
        } else {
            printf("\rError: Starting scan failed. Error code: %d\n", result);
            return -1;
        }
    }

    return 0;
}

// Stop discovery
int mp_bluetooth_gap_scan_stop(void) {
    printf("Scan stopping\r\n");
    wiced_result_t result;
    result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE, ctss_scan_result_cback);
    if (result != WICED_BT_SUCCESS) {
        mp_printf(&mp_plat_print, "Stopping scan failed with error: %d\n", result);
        return -1;
    }
    cyhal_timer_stop(&mp_bluetooth_gap_scan_timer_obj);
    cyhal_timer_free(&mp_bluetooth_gap_scan_timer_obj);

    mp_bluetooth_gap_on_scan_complete();
    return result;
}

// Connect to a found peripheral.
int mp_bluetooth_gap_peripheral_connect(uint8_t addr_type, const uint8_t *addr, int32_t duration_ms, int32_t min_conn_interval_us, int32_t max_conn_interval_us) {
    // ret_status = wiced_bt_gatt_le_connect( p_scan_result->remote_bd_addr, p_scan_result->ble_addr_type, BLE_CONN_MODE_LOW_DUTY, TRUE );
    // printf( "wiced_bt_gatt_connect status %d\r\n", ret_status );
    return 0;
}

// Cancel in-progress connection to a peripheral.
int mp_bluetooth_gap_peripheral_connect_cancel(void) {
    return 0;
}

#endif // MICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE


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

#if MICROPY_PY_BLUETOOTH_ENABLE_GATT_CLIENT

// Find all primary services on the connected peripheral.
int mp_bluetooth_gattc_discover_primary_services(uint16_t conn_handle, const mp_obj_bluetooth_uuid_t *uuid) {
    return 0;
}
// Find all characteristics on the specified service on a connected peripheral.
int mp_bluetooth_gattc_discover_characteristics(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle, const mp_obj_bluetooth_uuid_t *uuid) {
    return 0;
}
// Find all descriptors on the specified characteristic on a connected peripheral.
int mp_bluetooth_gattc_discover_descriptors(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle) {
    return 0;
}
// Initiate read of a value from the remote peripheral.
int mp_bluetooth_gattc_read(uint16_t conn_handle, uint16_t value_handle) {
    return 0;
}

// Write the value to the remote peripheral.
int mp_bluetooth_gattc_write(uint16_t conn_handle, uint16_t value_handle, const uint8_t *value, size_t value_len, unsigned int mode) {
    return 0;
}
// Initiate MTU exchange for a specific connection using the preferred MTU.
int mp_bluetooth_gattc_exchange_mtu(uint16_t conn_handle) {
    return 0;
}
#endif // MICROPY_PY_BLUETOOTH_ENABLE_GATT_CLIENT

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
