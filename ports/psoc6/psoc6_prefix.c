// psoc6_prefix.c becomes the initial portion of the generated pins file.

#include <stdio.h>

#include "py/obj.h"
#include "py/mphal.h"
#include "pins.h"

#define PIN(p_name, pin_addr, pin_name) \
    { \
        { &machine_pin_type }, \
        .pin_addr = pin_addr, \
        .pin_name = pin_name, \
    }
