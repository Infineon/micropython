import os
from machine import Pin

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    clk_pin = "P10_4"
    data_pin = "P10_5"
elif "CY8CPROTO-063-BLE" in board:
    print("SKIP")
    raise SystemExit
elif "CY8CKIT-062S2-AI" in board:
    clk_pin = "P10_4"
    data_pin = "P10_5"

start_time = time.time()

data_out = Pin("P13_7", mode=Pin.OUT, value=False)
clk_in = Pin("P13_6", Pin.IN, Pin.PULL_DOWN)

while time.time() - start_time < 10:  # Wait for 5 second
    data_out.low()
