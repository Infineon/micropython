# Machine - time_pulse_us test
# Setup: Connect pulse_in to pwm_pin

import time
import os
from machine import time_pulse_us
from machine import Pin
from machine import PWM

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pulse_in_pin = "P9_4"
    pwm_pin = "P9_6"
elif "CY8CPROTO-063-BLE" in board:
    pulse_in_pin = "P9_4"
    pwm_pin = "P9_6"
elif "CY8CKIT-062S2-AI" in board:
    pulse_in_pin = "P9_0"
    pwm_pin = "P9_5"

pulse_in = Pin(pulse_in_pin, Pin.IN)

pwm = PWM(pwm_pin, freq=1, duty_ns=250000000)
time.sleep(2)  # Wait for the pwm signal to be initialized and started
width = time_pulse_us(pulse_in, 1, 1000000)
print(
    f"Pulse timing verified for 25% dc : {True if (0.20 < (width / 1000000) < 0.30) else 'False, width=' + str(width / 1000000)}"
)

pwm.duty_ns(500000000)
width = time_pulse_us(pulse_in, 1, 1000000)
print(
    f"Pulse timing verified for 50% dc : {True if (0.45 < (width / 1000000) < 0.55) else 'False, width=' + str(width / 1000000)}"
)

pwm.duty_ns(750000000)
width = time_pulse_us(pulse_in, 1, 1000000)
print(
    f"Pulse timing verified for 75% dc : {True if (0.70 < (width / 1000000) < 0.80) else 'False, width=' + str(width / 1000000)}"
)

pulse_in.deinit()
pwm.deinit()
