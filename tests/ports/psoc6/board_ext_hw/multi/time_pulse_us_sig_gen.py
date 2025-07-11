import time
from machine import Pin
from machine import Timer
import os

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pulse_out_pin = "P9_4"
elif "CY8CPROTO-063-BLE" in board:
    pulse_out_pin = "P9_4"
elif "CY8CKIT-062S2-AI" in board:
    pulse_out_pin = "P9_0"

pulse_out = Pin(pulse_out_pin, Pin.OUT, Pin.PULL_DOWN)
pulse_out.value(0)
retry_times = 2
while True:
    for i in range(retry_times):
        pulse_out.toggle()
        time.sleep(2)
        retry_times = retry_times + 1
    break
