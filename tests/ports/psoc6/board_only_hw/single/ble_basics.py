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


def irq_handle(event, data):
    pass


ble_obj = bluetooth.BLE()
ble_obj.active(True)
print("BLE active: ", ble_obj.active())

# Config get test
print("\n** Configurations set for device **")
print("GAP_NAME: ", ble_obj.config("gap_name"))
print("MTU: ", ble_obj.config("mtu"))

ble_obj.config(addr_mode=0)  # Public
print("MAC: ", ble_obj.config("mac")[1].hex())

# Check if ble.irq registers handle
ble_obj.irq(irq_handle)

# Cannot be verified since trng is used and changes in every test
# ble_obj.config(addr_mode=1) #Static Random
# print("Set random address: ", ble_obj.config("mac")[1].hex())
print("\n")
ble_obj.active(False)
print("Turned off BLE radio: ", not ble_obj.active())
