
#include <stdio.h>
#include <string.h>


#include "py/runtime.h"
#include "py/mphal.h"

//TODO: never included mphalport.h but still compiles?
#include "modmachine.h"

// #define GPIO_IRQ_ALL (0xf)

// // Macros to access the state of the hardware.

// #define GPIO_IS_OUT(id) (sio_hw->gpio_oe & (1 << (id)))
// #define GPIO_IS_PULL_UP(id) (padsbank0_hw->io[(id)] & PADS_BANK0_GPIO0_PUE_BITS)
// #define GPIO_IS_PULL_DOWN(id) (padsbank0_hw->io[(id)] & PADS_BANK0_GPIO0_PDE_BITS)

// // Open drain behaviour is simulated.
// #define GPIO_IS_OPEN_DRAIN(id) (machine_pin_open_drain_mask & (1 << (id)))

// struct to hold pin information
typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint8_t id;
} machine_pin_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[CY_PROTO_062_4343_GPIOs] = {
    {{&machine_pin_type}, 0},  
    {{&machine_pin_type}, 1},
};

//GPIO array to map to CYHAL pins
cyhal_gpio_t CY_GPIO_array[CY_PROTO_062_4343_GPIOs] = {
    (P13_7), //GPIO 0 - LED
    (P0_4),  //GPIO 1 - BTN
};

//enum to hold pin modes 
enum {GPIO_MODE_IN, GPIO_MODE_OUT, GPIO_MODE_OPEN_DRAIN, GPIO_MODE_ALT, GPIO_MODE_ALT_OPEN_DRAIN};

//enum to hold pin drive strengths 
enum {GPIO_DRIVE_CAP_0, GPIO_DRIVE_CAP_1, GPIO_DRIVE_CAP_2, GPIO_DRIVE_CAP_3};

//enum to hold pulls
enum {GPIO_PULL_UP, GPIO_PULL_DOWN};

//enum for alt functions
enum {HSIOM_GPIO_FUNC}; // see file gpio_psoc6_02_124_bga.h

//enums and structs to handle ARGS in MPY
enum {ARG_mode, ARG_pull, ARG_value, ARG_drive, ARG_alt};

static const mp_arg_t allowed_args[] = {
    {MP_QSTR_mode,  MP_ARG_OBJ,                  {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_pull,  MP_ARG_OBJ,                  {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_drive, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_alt,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = HSIOM_GPIO_FUNC}},
};

void machine_pin_init(void) {
    mp_printf(&mp_plat_print, "machine pin init\n");
    //memset(MP_STATE_PORT(machine_pin_irq_obj), 0, sizeof(MP_STATE_PORT(machine_pin_irq_obj)));
    //irq_add_shared_handler(IO_IRQ_BANK0, gpio_irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    //irq_set_enabled(IO_IRQ_BANK0, true);

    //TODO: maybe the cybsp_init() can be called here
}

//print method of Pin class
static void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(&mp_plat_print, "machine pin print\n");
    machine_pin_obj_t *self = self_in;

    en_hsiom_sel_t pin_func = PIN_GET_HSIOM_FUNC(self->id);
    qstr mode_qstr = MP_QSTR_None; //TODO: compare with rp2, init value needed here due to "-werror=maybe-uninitialized"
    qstr pull_qstr = MP_QSTR_None;
    uint8_t pin_value = -1;

    if (pin_func == HSIOM_SEL_GPIO){
        if (GPIO_IS_OPEN_DRAIN(self->id)){
            mode_qstr = MP_QSTR_OPEN_DRAIN;
        }
        else if (GPIO_IS_OUT(self->id)){
            mode_qstr = MP_QSTR_OUT;
        }
        else if (GPIO_IS_IN(self->id)){
            mode_qstr = MP_QSTR_IN;
        }
        else{ //only pull up and pull down are prescribed in MPY docs
            if (GPIO_IS_PULL_UP(self->id)){
                pull_qstr = MP_QSTR_PULL_UP;
                if (GPIO_GET_CYPDL_DRIVE(self->id) < 8) //drive enum is less than 8 when input buffer is off (or pin is cfgd as output)
                    mode_qstr = MP_QSTR_OUT;
                else
                    mode_qstr = MP_QSTR_IN;
            }
            else if (GPIO_IS_PULL_DOWN(self->id)){
                pull_qstr = MP_QSTR_PULL_DOWN;
                if (GPIO_GET_CYPDL_DRIVE(self->id) < 8)
                    mode_qstr = MP_QSTR_OUT;
                else
                    mode_qstr = MP_QSTR_IN;
            }
            else
                mp_printf(print,"no params set for given pin object"); 
        }
        pin_value = GPIO_GET_VALUE(self->id);    
    } 
    else{
        mode_qstr = MP_QSTR_ALT; 
    }

    mp_printf(print, "Pin:%u, Mode=%q, Pull=%q, Value=%u", self->id, mode_qstr, pull_qstr, pin_value);        
}

void machine_pin_deinit(void) {
    mp_printf(&mp_plat_print, "machine pin deinit\n");
}

//helper function to invoke HAL-level GPIO functions
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // checks for args since many combinations are not desirable!

    // check for initial value of pin
    bool value;
    if (args[ARG_value].u_obj != mp_const_none)
        value = mp_obj_is_true(args[ARG_value].u_obj); //pins are active low
    else
        value = 1; // initially hold pin high (LED inactive)

    cyhal_gpio_direction_t direction = CYHAL_GPIO_DIR_OUTPUT; // initially set as output
    cyhal_gpio_drive_mode_t drive = CYHAL_GPIO_DRIVE_PULLUPDOWN; // initially set both pull up-down (safe for both IN-OUT)
    //Note: cyhal drive modes are in an enum here: cyhal_gpio.h; also described in the header
    // check for direction and set drive accordingly
    if (args[ARG_mode].u_obj != mp_const_none) {
        mp_int_t mode = mp_obj_get_int(args[ARG_mode].u_obj);
        if (mode == GPIO_MODE_IN) {
            direction = CYHAL_GPIO_DIR_INPUT;
            if (args[ARG_pull].u_obj == mp_const_none) // if pull not provided, set pull as NONE for input (recommended)
                drive = CYHAL_GPIO_DRIVE_PULL_NONE; 
        } else if (mode == GPIO_MODE_OUT) {
            direction = CYHAL_GPIO_DIR_OUTPUT;
            if (args[ARG_pull].u_obj == mp_const_none) // if pull not provided, set pull as STRONG for output (recommended)
                drive = CYHAL_GPIO_DRIVE_STRONG;
        } else if (mode == GPIO_MODE_OPEN_DRAIN) {
            drive = CYHAL_GPIO_DRIVE_OPENDRAINDRIVESLOW; //covers both the cases of open_drain -> strong 0 when value=0, highZ when value=1

            //see constructor here: https://docs.micropython.org/en/latest/library/machine.Pin.html
            //also drive modes explained here: https://community.infineon.com/t5/Knowledge-Base-Articles/Drive-Modes-in-PSoC-GPIO/ta-p/248470
            //see app note: https://www.infineon.com/dgdl/Infineon-AN2094_PSoC_1_Getting_Started_With_GPIO-ApplicationNotes-v12_00-EN.pdf?fileId=8ac78c8c7cdc391c017d072966174e13&utm_source=cypress&utm_medium=referral&utm_campaign=202110_globe_en_all_integration-application_note
            
            //Note: below code is redundant since definition of open drain is later understood.
            // if(value == 1)
            //     drive = CYHAL_GPIO_DRIVE_NONE; // Hi-Z as per MPY docs
            // else if (value == 0)
            //     drive = CYHAL_GPIO_DRIVE_OPENDRAINDRIVESLOW; // drive low as per MPY docs
            // else
            //     mp_printf(&mp_plat_print, "Specify init value of pin for open-drain\n"); //never reached since init val assumed above, may be removed    
        } else {
            // Alternate function.
            mp_printf(&mp_plat_print, "Not implemented!!!\n");
        }
    }

    // check for drive strength
    if (args[ARG_drive].u_obj != mp_const_none) {
        mp_printf(&mp_plat_print, "CYHAL has only one drive strength for output mode.\n"); //see Cy_GPIO_GetDriveSel() for PDL drive modes
    }

    // check for pulls
    uint32_t pull = 0;
    if (args[ARG_pull].u_obj != mp_const_none) {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
        if (pull == GPIO_PULL_UP)
            drive = CYHAL_GPIO_DRIVE_PULLUP; //TODO: should we also force the value to 1 in case of pull up
        else if (pull == GPIO_PULL_DOWN)
            drive = CYHAL_GPIO_DRIVE_PULLDOWN; //TODO: should we also force the value to 0 in case of pull down
        else
            mp_raise_ValueError(MP_ERROR_TEXT("invalid value of pull"));
    }

    // check for alt function
    if(args[ARG_alt].u_obj != mp_const_none)
    if (args[ARG_alt].u_int != HSIOM_GPIO_FUNC) {
        mp_raise_ValueError(MP_ERROR_TEXT("Alternate functions are not implemented!!!"));
    }
    // TODO: check for ALT configs of pins. HSIOM configs
    // see line 569 onwards in gpio_psoc6_02_124_bga.h 

    //cy_rslt_t result = cybsp_init(); //returns ERROR since already called in MPY main
    
    // call the pin init functions with params gathered above
    cy_rslt_t result = cyhal_gpio_init(CY_GPIO_array[self->id], direction, drive, value);

    if (result != CY_RSLT_SUCCESS)
         mp_printf(&mp_plat_print, "GPIO set error\n");

    //TODO: remove this later, only for DEBUG to see what params are going into GPIO init
    // params can be found in enums here: cyhal_gpio.h
    mp_printf(&mp_plat_print, "Dir: %d, Drive:%d, Value:%d\n", direction, drive, value);
    
    return mp_const_none;
}

// constructor(id,mode,pull,value=value,drive=drive,alt=alt)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    
    mp_printf(&mp_plat_print, "%q constructor invoked\n", MP_QSTR_Pin);
    
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    //object ptr for the class obj instantiated
    const machine_pin_obj_t *self = NULL;
    
    // to be used if we have named pins
    // if (mp_obj_is_str(args[0])) {
    //     const char *name = mp_obj_str_get_str(args[0]);
       
    //     if (!self) {
    //         mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unknown named pin \"%s\""), name);
    //     }
    // }

    if (!self) {
        // get the wanted pin object
        int wanted_pin = mp_obj_get_int(args[0]);
        if (!(0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj))) {
            mp_raise_ValueError(MP_ERROR_TEXT("invalid pin"));
        }
        self = &machine_pin_obj[wanted_pin];
    }
    // note we have different init args based on the type of pin. so Pin("LED", Pin.OUT) may not always make sense
   
    // go into param arg parsing if args apart from "id" are provided (for ex. pin.Mode, pin.PULL etc)
    if (n_args > 1 || n_kw > 0) {
        mp_printf(&mp_plat_print, "init helper function called\n"); //TODO: remove this later, only for DEBUG
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args); //skipping "id" as an arg.
    }
    return MP_OBJ_FROM_PTR(self);
};


STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_printf(&mp_plat_print, "machine pin call\n");
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;

    //Note: see https://docs.micropython.org/en/latest/library/machine.Pin.html#machine.Pin.value
    if (n_args == 0) {
        // get pin value if pin is in input mode
        return GPIO_GET_VALUE_CALL(self->id);    
    }
    else {
        // set pin
        bool value = mp_obj_is_true(args[0]);
        if (GPIO_IS_IN(self->id) || GPIO_IS_OUT(self->id) || GPIO_IS_OPEN_DRAIN(self->id)){ //set the output buffer of output driver with given value; 
            if (value)                                //if Pin.Mode is Pin.IN, value will reflect when pin is next set as output.     
                GPIO_SET_VALUE(self->id);
            else
                GPIO_CLR_VALUE(self->id);
        }
    } //given how the PSoC architecture is, if the "drive mode" is set correctly, the same set/clr functions can be used for all the "modes".
    //refer pg 245 of PSoC6 Arch TRM: https://www.infineon.com/dgdl/Infineon-PSoC_6_MCU_PSoC_62_Architecture_Technical_Reference_Manual-AdditionalTechnicalInformation-v08_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f94758401d1&utm_source=cypress&utm_medium=referral&utm_campaign=202110_globe_en_all_integration-files

    return mp_const_none;
 }

//instantiates obj of Pin class
//Pin.init(mode,pull,value=value,drive=drive,alt=alt)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    mp_printf(&mp_plat_print, "Pin class object instantiated\n"); //TODO: remove later
    //redirect args except self to the obj init helper function
    //args[0] here is self ptr
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_obj_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// Pin.toggle()
STATIC mp_obj_t machine_pin_toggle(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (GPIO_IS_IN(self->id) || GPIO_IS_OUT(self->id) || GPIO_IS_OPEN_DRAIN(self->id)){ //toggle the output buffer of output driver with given value;
        GPIO_TOGGLE_VALUE(self->id); //for output it takes effect instantly; for input pins, the effect will show when
                                     //pin is set as input next. For open drain, behavior shifts between 0 and HiZ.   
    }
    return mp_const_none;
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_toggle_obj, machine_pin_toggle);

// Pin.high()
STATIC mp_obj_t machine_pin_high(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (GPIO_IS_IN(self->id) || GPIO_IS_OUT(self->id) || GPIO_IS_OPEN_DRAIN(self->id)){ //toggle the output buffer of output driver with given value;
        GPIO_SET_VALUE(self->id); //for output it takes effect instantly; for input pins, the effect will show when
                                     //pin is set as input next. For open drain, behavior shifts between 0 and HiZ.   
    }            
    return mp_const_none;
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_high_obj, machine_pin_high);

// Pin.low()
STATIC mp_obj_t machine_pin_low(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (GPIO_IS_IN(self->id) || GPIO_IS_OUT(self->id) || GPIO_IS_OPEN_DRAIN(self->id)){ //toggle the output buffer of output driver with given value;
        GPIO_CLR_VALUE(self->id); //for output it takes effect instantly; for input pins, the effect will show when
                                     //pin is set as input next. For open drain, behavior shifts between 0 and HiZ.   
    }            
    return mp_const_none;
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_low_obj, machine_pin_low);

// // Pin.Mode()
// STATIC mp_obj_t machine_pin_mode(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
//     mp_printf(&mp_plat_print,"machine_pin_mode: Not implemented!");
//     mp_arg_check_num(n_args, n_kw, 0, 1, false);
//     //machine_pin_obj_t *self = self_in;

//     // if (n_args == 0) {
//     //     //get mode
//     // }
//     // else {
//     //    //set mode
//     // }

//     return mp_const_none;
    
// }

// STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_mode_obj,1,2, machine_pin_mode);


STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init),  MP_ROM_PTR(&machine_pin_obj_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&machine_pin_toggle_obj) },
    { MP_ROM_QSTR(MP_QSTR_low), MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_high), MP_ROM_PTR(&machine_pin_high_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_high_obj) },
    //{ MP_ROM_QSTR(MP_QSTR_mode), MP_ROM_PTR(&machine_pin_mode_obj) }, //TODO: got unknown error! 

    // class constants 
    //pin.Mode
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_IN) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_OUT) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_OPEN_DRAIN) },
    { MP_ROM_QSTR(MP_QSTR_ALT_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_ALT_OPEN_DRAIN) },

    //pin.drive
    { MP_ROM_QSTR(MP_QSTR_DRIVE_0), MP_ROM_INT(GPIO_DRIVE_CAP_0) },
    { MP_ROM_QSTR(MP_QSTR_DRIVE_1), MP_ROM_INT(GPIO_DRIVE_CAP_1) },
    { MP_ROM_QSTR(MP_QSTR_DRIVE_2), MP_ROM_INT(GPIO_DRIVE_CAP_2) },
    { MP_ROM_QSTR(MP_QSTR_DRIVE_3), MP_ROM_INT(GPIO_DRIVE_CAP_3) },

    //pin.pull
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_PULL_UP) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULL_DOWN) },

    //pin.alt
    { MP_ROM_QSTR(MP_QSTR_ALT), MP_ROM_INT(GPIO_MODE_ALT) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    machine_pin_type,
    MP_QSTR_Pin,
    MP_TYPE_FLAG_NONE,
    make_new, mp_pin_make_new,
    print, machine_pin_print,
    call, machine_pin_call,
    locals_dict, &machine_pin_locals_dict
    );
