import os

board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    print("SKIP")
    raise SystemExit
elif "CY8CPROTO-063-BLE" in board:
    import bluetooth
elif "CY8CKIT-062S2-AI" in board:
    print("SKIP")
    raise SystemExit


ble_obj = bluetooth.BLE()
