import machine
import time
import os

""" Note: Run this test only after wdt.py tests to check if a Watchdog reset has occurred """

print("\n***** Test 1: Check if WDT triggered reset *****\n")

print("machine.reset_cause(): ", machine.reset_cause())
if machine.reset_cause() == machine.WDT_RESET:
    print("PASS")
else:
    print(machine.reset_cause())
    print("FAIL")
