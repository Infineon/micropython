#ifndef MICROPY_INCLUDED_PSOC6_PINS_H
#define MICROPY_INCLUDED_PSOC6_PINS_H


// port-specific includes
#include "modmachine.h"


// cy includes
#include "cyhal.h"

// Add all machine pin objects - GPIO , I2C, ADC etc.
typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t pin_addr;
    char *pin_name;
} machine_pin_obj_t;

#include "genhdr/pins.h"

// Function Prototypes to support interaction between c<->py
int pin_find(mp_obj_t obj);
int pin_addr_by_name(mp_obj_t obj);

extern const mp_obj_type_t pin_cpu_pins_obj_type;
extern const mp_obj_dict_t pin_cpu_pins_locals_dict;

/* TODO: to auto-generate the pin instances in the pin_obj array
and thereby capture the obj count. It cannot be declared without a defined size,
since there are sizeof() and other functions in machine_pin which act on this array
and need a deterministic size of the array to compile. */
extern const machine_pin_obj_t machine_pin_obj[];


#endif // MICROPY_INCLUDED_PSOC6_PINS_H
