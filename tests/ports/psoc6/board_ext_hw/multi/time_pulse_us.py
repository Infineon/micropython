import time
import os
from machine import time_pulse_us
from machine import Pin


# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pulse_in_pin = "P9_4"
elif "CY8CPROTO-063-BLE" in board:
    pulse_in_pin = "P9_4"
elif "CY8CKIT-062S2-AI" in board:
    pulse_in_pin = "P9_0"


pulse_in = Pin(pulse_in_pin, Pin.IN)
print("Pin.IN value before: ", pulse_in.value())
width = time_pulse_us(pulse_in, 1, 15000000)  # Wait for high pulse, timeout=2 seconds
time.sleep(1)
print("Pin.IN value after: ", pulse_in.value())
print("Pulse timing(in us): ", width)
