from machine import Pin, time_pulse_us
import os
import time

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pulse_in_pin = "P9_7"
    ack_out_pin = "P9_6"
elif "CY8CPROTO-063-BLE" in board:
    pulse_in_pin = "P9_4"
    ack_out_pin = "P9_6"
elif "CY8CKIT-062S2-AI" in board:
    pulse_in_pin = "P9_7"
    ack_out_pin = "P9_6"

pin_high_received = False


def cback(pin):
    global pin_high_received
    pin_high_received = True


def blocking_delay_ms(delay_ms):
    start = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), start) < delay_ms:
        pass


pulse_in = Pin(pulse_in_pin, Pin.IN, Pin.PULL_DOWN)
ack_out = Pin(ack_out_pin, Pin.OUT)
ack_out.low()

width = 0

pulse_in.irq(trigger=Pin.IRQ_RISING, handler=cback)

# Send begin ack to start generating pulse
ack_out.high()
blocking_delay_ms(1000)
ack_out.low()

# Wait to receive pulse high signal
while not pin_high_received:
    pass

# Send pulse high recvd ack
ack_out.high()

# Measure the pulse width
width = time_pulse_us(pulse_in, 1, 10000000)

print(
    f"Pulse timing verified: {True if (0.98 < (width / 1000000) < 1.2) else 'False, width=' + str(width / 1000000)}"
)

pulse_in.deinit()
ack_out.deinit()
