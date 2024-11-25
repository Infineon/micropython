import os
from machine import PDM_PCM, Pin
import machine

# Allocate pin based on board
board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    clk_pin = "P10_4"
    data_pin = "P10_5"
    send_signal_to_tx_pin = "P10_0"

elif "CY8CPROTO-063-BLE" in board:
    print("SKIP")
    raise SystemExit
elif "CY8CKIT-062S2-AI" in board:
    clk_pin = "P10_4"
    data_pin = "P10_5"


def generate_exp_seq(data):
    exp_seq = bytearray(data * 64)
    return exp_seq


# Test Case 1: Blocking mode
_sampling_rate = [8000, 16000, 32000, 48000, 22050, 44100]
_mode = [PDM_PCM.MONO_LEFT, PDM_PCM.MONO_RIGHT, PDM_PCM.STEREO]
_bits = 16
exp_data = [[0xFF]]
iterations = 100
exp_seq = generate_exp_seq([0x00])

for i in range(len(_sampling_rate)):  # Check for all sampling rates
    set_sampling_rate = _sampling_rate[i]
    if _sampling_rate[i] == 22050 or _sampling_rate[i] == 44100:
        machine.freq(machine.AUDIO_PDM_22_579_000_HZ)
    else:
        machine.freq(machine.AUDIO_PDM_24_576_000_HZ)
    for j in range(len(_mode)):  # Check for all modes
        set_mode = _mode[j]
        if set_mode == PDM_PCM.STEREO:
            iterations = 200
        pdm_pcm = PDM_PCM(
            0,
            sck=clk_pin,
            data=data_pin,
            sample_rate=set_sampling_rate,
            decimation_rate=64,
            bits=PDM_PCM.BITS_16,
            format=set_mode,
            left_gain=0,
            right_gain=0,
        )
        pdm_pcm.init()  # Start

        for k in range(iterations):
            rx_buf = bytearray([0] * 64)
            num_read = pdm_pcm.readinto(rx_buf)
            # print("buf :", ''.join(f'{x:02x} ' for x in rx_buf))
            if rx_buf[:3] == bytearray([exp_data[0][0], exp_data[0][0], exp_data[0][0]]):
                is_seq_received = rx_buf == exp_seq
                if is_seq_received:
                    print(
                        f"data high received for mode = {set_mode}, bits = PDM_PCM.BITS_16, rate = {set_sampling_rate} : {is_seq_received}"
                    )
                    break
        pdm_pcm.deinit()
