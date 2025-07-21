from machine import Pin
import os
import time

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    pulse_out_pin = "P9_7"
    ack_in_pin = "P9_6"
elif "CY8CPROTO-063-BLE" in board:
    pulse_out_pin = "P9_4"
    ack_in_pin = "P9_6"
elif "CY8CKIT-062S2-AI" in board:
    pulse_out_pin = "P9_7"
    ack_in_pin = "P9_6"

ack_recvd = False


def cback(pin):
    global ack_recvd
    ack_recvd = True


def blocking_delay_ms(delay_ms):
    start = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), start) < delay_ms:
        pass


pulse_out = Pin(pulse_out_pin, Pin.OUT, value=False)
ack_in = Pin(ack_in_pin, Pin.IN)
ack_in.irq(trigger=Pin.IRQ_RISING, handler=cback)

# Starting condition always low pulse
pulse_out.low()
blocking_delay_ms(500)

while not ack_recvd:
    pass  # Wait for the Begin ACK signal

ack_recvd = False

# Generate high pulse
pulse_out.high()
while not ack_recvd:
    pass  # Wait for high received ACK signal

# Wait for 1sec to time the pulse
blocking_delay_ms(1000)

pulse_out.low()

pulse_out.deinit()  # Deinitialize the pulse output pin
ack_in.deinit()
