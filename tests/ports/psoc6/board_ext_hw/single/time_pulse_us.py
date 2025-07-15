# Machine - time_pulse_us test
# Setup: Connect pulse_in to pwm_pin

from machine import PWM, Pin, time_pulse_us
import os
import time

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pwm_pin = "P13_7"
    pulse_pin_in = "P13_6"
elif "CY8CPROTO-063-BLE" in board:
    pwm_pin = "P12_6"
    pulse_pin_in = "P12_7"
elif "CY8CKIT-062S2-AI" in board:
    pwm_pin = "P9_6"
    pulse_pin_in = "P9_7"

pulse_in = Pin(pulse_pin_in, Pin.IN)

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
