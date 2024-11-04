import os
from machine import PDM_PCM, Pin

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

rx_buf = bytearray([0] * 4096)

print("PDM_PCM Tests")
pdm_pcm = PDM_PCM(
    0,
    sck=clk_pin,
    data=data_pin,
    sample_rate=8000,
    decimation_rate=64,
    bits=PDM_PCM.BITS_16,
    format=PDM_PCM.STEREO,
    left_gain=0,
    right_gain=0,
)

pdm_pcm.init()

# Read PCM data
num_read = pdm_pcm.readinto(rx_buf)
print("num bytes read: ", num_read)
print("Read buff values: ", rx_buf)

pdm_pcm.deinit()
