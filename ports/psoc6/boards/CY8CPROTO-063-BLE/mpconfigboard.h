// Board and hardware specific configuration
#define MICROPY_HW_MCU_NAME                     "PSoC63"
#define MICROPY_HW_BOARD_NAME                   "CY8CPROTO-063-BLE"

#define MICROPY_PY_NETWORK_HOSTNAME_DEFAULT     "CY8C-063-BLE"

#define MICROPY_GC_HEAP_SIZE                    (64 * 1024) // 64 KB

#define MICROPY_PY_MACHINE_SPI_SLAVE            (1)

// Board specific configurations
#define MAX_UART             2 
#define MAX_TIMER (32)
#define MAX_SPI              10 
#define MAX_I2C              10 
#define MAX_PWM_OBJS 10 
#define MICROPY_HW_MAX_I2S 2 
