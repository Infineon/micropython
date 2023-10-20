
#include "py/mphal.h"
#include "machine_pin_phy.h"

// extern const uint8_t machine_pin_num_of_cpu_pins;

/*machine_pin_phy_obj_t machine_pin_phy_obj[] = {
    {PIN_P0_0, "P0_0",  PIN_PHY_FUNC_NONE},
    {PIN_P0_1, "P0_1",  PIN_PHY_FUNC_NONE},
    {PIN_P0_2, "P0_2",  PIN_PHY_FUNC_NONE},
    {PIN_P0_3, "P0_3",  PIN_PHY_FUNC_NONE},
    {PIN_P0_4, "P0_4",  PIN_PHY_FUNC_NONE},
    {PIN_P0_5, "P0_5",  PIN_PHY_FUNC_NONE},

    {PIN_P1_0, "P1_0",  PIN_PHY_FUNC_NONE},
    {PIN_P1_1, "P1_1",  PIN_PHY_FUNC_NONE},
    {PIN_P1_2, "P1_2",  PIN_PHY_FUNC_NONE},
    {PIN_P1_3, "P1_3",  PIN_PHY_FUNC_NONE},
    {PIN_P1_4, "P1_4",  PIN_PHY_FUNC_NONE},
    {PIN_P1_5, "P1_5",  PIN_PHY_FUNC_NONE},

    {PIN_P2_0, "P2_0",  PIN_PHY_FUNC_NONE},
    {PIN_P2_1, "P2_1",  PIN_PHY_FUNC_NONE},
    {PIN_P2_2, "P2_2",  PIN_PHY_FUNC_NONE},
    {PIN_P2_3, "P2_3",  PIN_PHY_FUNC_NONE},
    {PIN_P2_4, "P2_4",  PIN_PHY_FUNC_NONE},
    {PIN_P2_5, "P2_5",  PIN_PHY_FUNC_NONE},
    {PIN_P2_6, "P2_6",  PIN_PHY_FUNC_NONE},
    {PIN_P2_7, "P2_7",  PIN_PHY_FUNC_NONE},

    {PIN_P3_0, "P3_0",  PIN_PHY_FUNC_NONE},
    {PIN_P3_1, "P3_1",  PIN_PHY_FUNC_NONE},
    {PIN_P3_2, "P3_2",  PIN_PHY_FUNC_NONE},
    {PIN_P3_3, "P3_3",  PIN_PHY_FUNC_NONE},
    {PIN_P3_4, "P3_4",  PIN_PHY_FUNC_NONE},
    {PIN_P3_5, "P3_5",  PIN_PHY_FUNC_NONE},

    {PIN_P4_0, "P4_0",  PIN_PHY_FUNC_NONE},
    {PIN_P4_1, "P4_1",  PIN_PHY_FUNC_NONE},

    {PIN_P5_0, "P5_0",  PIN_PHY_FUNC_NONE},
    {PIN_P5_1, "P5_1",  PIN_PHY_FUNC_NONE},
    {PIN_P5_2, "P5_2",  PIN_PHY_FUNC_NONE},
    {PIN_P5_3, "P5_3",  PIN_PHY_FUNC_NONE},
    {PIN_P5_4, "P5_4",  PIN_PHY_FUNC_NONE},
    {PIN_P5_5, "P5_5",  PIN_PHY_FUNC_NONE},
    {PIN_P5_6, "P5_6",  PIN_PHY_FUNC_NONE},
    {PIN_P5_7, "P5_7",  PIN_PHY_FUNC_NONE},

    {PIN_P6_0, "P6_0",  PIN_PHY_FUNC_NONE},
    {PIN_P6_1, "P6_1",  PIN_PHY_FUNC_NONE},
    {PIN_P6_2, "P6_2",  PIN_PHY_FUNC_NONE},
    {PIN_P6_3, "P6_3",  PIN_PHY_FUNC_NONE},
    {PIN_P6_4, "P6_4",  PIN_PHY_FUNC_NONE},
    {PIN_P6_5, "P6_5",  PIN_PHY_FUNC_NONE},
    {PIN_P6_6, "P6_6",  PIN_PHY_FUNC_NONE},
    {PIN_P6_7, "P6_7",  PIN_PHY_FUNC_NONE},

    {PIN_P7_0, "P7_0",  PIN_PHY_FUNC_NONE},
    {PIN_P7_1, "P7_1",  PIN_PHY_FUNC_NONE},
    {PIN_P7_2, "P7_2",  PIN_PHY_FUNC_NONE},
    {PIN_P7_3, "P7_3",  PIN_PHY_FUNC_NONE},
    {PIN_P7_4, "P7_4",  PIN_PHY_FUNC_NONE},
    {PIN_P7_5, "P7_5",  PIN_PHY_FUNC_NONE},
    {PIN_P7_6, "P7_6",  PIN_PHY_FUNC_NONE},
    {PIN_P7_7, "P7_7",  PIN_PHY_FUNC_NONE},

    {PIN_P8_0, "P8_0",  PIN_PHY_FUNC_NONE},
    {PIN_P8_1, "P8_1",  PIN_PHY_FUNC_NONE},
    {PIN_P8_2, "P8_2",  PIN_PHY_FUNC_NONE},
    {PIN_P8_3, "P8_3",  PIN_PHY_FUNC_NONE},
    {PIN_P8_4, "P8_4",  PIN_PHY_FUNC_NONE},
    {PIN_P8_5, "P8_5",  PIN_PHY_FUNC_NONE},
    {PIN_P8_6, "P8_6",  PIN_PHY_FUNC_NONE},
    {PIN_P8_7, "P8_7",  PIN_PHY_FUNC_NONE},

    {PIN_P9_0, "P9_0",  PIN_PHY_FUNC_NONE},
    {PIN_P9_1, "P9_1",  PIN_PHY_FUNC_NONE},
    {PIN_P9_2, "P9_2",  PIN_PHY_FUNC_NONE},
    {PIN_P9_3, "P9_3",  PIN_PHY_FUNC_NONE},
    {PIN_P9_4, "P9_4",  PIN_PHY_FUNC_NONE},
    {PIN_P9_5, "P9_5",  PIN_PHY_FUNC_NONE},
    {PIN_P9_6, "P9_6",  PIN_PHY_FUNC_NONE},
    {PIN_P9_7, "P9_7",  PIN_PHY_FUNC_NONE},

    {PIN_P10_0, "P10_0",  PIN_PHY_FUNC_NONE},
    {PIN_P10_1, "P10_1",  PIN_PHY_FUNC_NONE},
    {PIN_P10_2, "P10_2",  PIN_PHY_FUNC_NONE},
    {PIN_P10_3, "P10_3",  PIN_PHY_FUNC_NONE},
    {PIN_P10_4, "P10_4",  PIN_PHY_FUNC_NONE},
    {PIN_P10_5, "P10_5",  PIN_PHY_FUNC_NONE},
    {PIN_P10_6, "P10_6",  PIN_PHY_FUNC_NONE},
    {PIN_P10_7, "P10_7",  PIN_PHY_FUNC_NONE},

    {PIN_P11_0, "P11_0",  PIN_PHY_FUNC_NONE},
    {PIN_P11_1, "P11_1",  PIN_PHY_FUNC_NONE},
    {PIN_P11_2, "P11_2",  PIN_PHY_FUNC_NONE},
    {PIN_P11_3, "P11_3",  PIN_PHY_FUNC_NONE},
    {PIN_P11_4, "P11_4",  PIN_PHY_FUNC_NONE},
    {PIN_P11_5, "P11_5",  PIN_PHY_FUNC_NONE},
    {PIN_P11_6, "P11_6",  PIN_PHY_FUNC_NONE},
    {PIN_P11_7, "P11_7",  PIN_PHY_FUNC_NONE},

    {PIN_P12_0, "P12_0",  PIN_PHY_FUNC_NONE},
    {PIN_P12_1, "P12_1",  PIN_PHY_FUNC_NONE},
    {PIN_P12_2, "P12_2",  PIN_PHY_FUNC_NONE},
    {PIN_P12_3, "P12_3",  PIN_PHY_FUNC_NONE},
    {PIN_P12_4, "P12_4",  PIN_PHY_FUNC_NONE},
    {PIN_P12_5, "P12_5",  PIN_PHY_FUNC_NONE},
    {PIN_P12_6, "P12_6",  PIN_PHY_FUNC_NONE},
    {PIN_P12_7, "P12_7",  PIN_PHY_FUNC_NONE},

    {PIN_P13_0, "P13_0",  PIN_PHY_FUNC_NONE},
    {PIN_P13_1, "P13_1",  PIN_PHY_FUNC_NONE},
    {PIN_P13_2, "P13_2",  PIN_PHY_FUNC_NONE},
    {PIN_P13_3, "P13_3",  PIN_PHY_FUNC_NONE},
    {PIN_P13_4, "P13_4",  PIN_PHY_FUNC_NONE},
    {PIN_P13_5, "P13_5",  PIN_PHY_FUNC_NONE},
    {PIN_P13_6, "P13_6",  PIN_PHY_FUNC_NONE},
    {PIN_P13_7, "P13_7",  PIN_PHY_FUNC_NONE},

    {PIN_USBDP, "USBDP",  PIN_PHY_FUNC_NONE},
    {PIN_USBDM, "USBDM",  PIN_PHY_FUNC_NONE}
};*/

// Function definitions
// helper function to translate pin_name(string) into machine_pin_io_obj_t index.
int pin_find(mp_obj_t pin) {
    int wanted_pin = -1;
    if (mp_obj_is_small_int(pin)) {
        int pin_addr = mp_obj_get_int(pin);
        for (int i = 0; i < machine_pin_num_of_cpu_pins; i++) {
            if (machine_pin_phy_obj[i].addr == pin_addr) {
                wanted_pin = i;
                break;
            }
        }
    } else if (mp_obj_is_str(pin)) {
        // Search by name
        size_t slen;
        const char *s = mp_obj_str_get_data(pin, &slen);
        for (int i = 0; i < machine_pin_num_of_cpu_pins; i++) {
            if (slen == strlen(machine_pin_phy_obj[i].name) && strncmp(s, machine_pin_phy_obj[i].name, slen) == 0) {
                wanted_pin = i;
                break;
            }
        }
    }

    if (!(0 <= wanted_pin && wanted_pin < machine_pin_num_of_cpu_pins)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin: Pin not defined!"));
    }

    return wanted_pin;
}

mp_obj_t pin_name_by_addr(mp_obj_t pin) {
    if (mp_obj_is_int(pin)) {
        const char *name = machine_pin_phy_obj[pin_find(pin)].name;
        return mp_obj_new_str(name, strlen(name));
    } else {
        return NULL; // expecting a int as input
    }
}

// helper function to translate pin_name(string) into machine_pin_io_obj_t->pin_addr
int pin_addr_by_name(mp_obj_t pin) {
    if (mp_obj_is_str(pin)) {
        return machine_pin_phy_obj[pin_find(pin)].addr;
    } else {
        return -1; // expecting a str as input
    }
}

#define pin_phy_assert_null(x) { if (x == NULL) { return NULL; } }

machine_pin_phy_obj_t *pin_phy_find_by_name(mp_obj_t pin_name) {
    size_t slen;
    const char *s = mp_obj_str_get_data(pin_name, &slen);
    for (int i = 0; i < machine_pin_num_of_cpu_pins; i++) {
        if (slen == strlen(machine_pin_phy_obj[i].name) && strncmp(s, machine_pin_phy_obj[i].name, slen) == 0) {
            return &machine_pin_phy_obj[i];
        }
    }
    mp_raise_ValueError(MP_ERROR_TEXT("invalid pin: Pin not defined!"));
    return NULL;
}

bool pin_phy_is_alloc(machine_pin_phy_obj_t *pin_phy) {
    if (pin_phy->allocated_func != PIN_PHY_FUNC_NONE) {
        return true;
    }
    return false;
}

machine_pin_phy_obj_t *pin_phy_alloc(mp_obj_t pin_name, machine_pin_phy_func_t func) {
    machine_pin_phy_obj_t *pin_phy = pin_phy_find_by_name(pin_name);
    pin_phy_assert_null(pin_phy);

    pin_phy->allocated_func = func;

    return pin_phy;
}

machine_pin_phy_obj_t *pin_phy_realloc(mp_obj_t pin_name, machine_pin_phy_func_t func) {
    machine_pin_phy_obj_t *pin_phy = pin_phy_find_by_name(pin_name);
    pin_phy_assert_null(pin_phy);

    if (pin_phy_is_alloc(pin_phy)) {
        pin_phy_free(pin_phy);
    }

    pin_phy->allocated_func = func;

    return pin_phy;
}

void pin_phy_free(machine_pin_phy_obj_t *pin_phy) {
    pin_phy->allocated_func = PIN_PHY_FUNC_NONE;
}


void mod_pin_phy_deinit(void) {
    for (int i = 0; i < machine_pin_num_of_cpu_pins; i++) {
        pin_phy_free(&machine_pin_phy_obj[i]);
    }
}