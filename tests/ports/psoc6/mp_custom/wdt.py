"""import machine
import time

# invalid test cases
print("\n***** Test 1: Wrong WDT id *****\n")
try:
    wdt = machine.WDT(1, timeout=6000)
except Exception:
    print("FAIL")

print("\n***** Test 2: WDT timeout too low *****\n")
try:
    wdt = machine.WDT(0, 0)
except Exception:
    print("FAIL")

print("\n***** Test 3: WDT timeout too high *****\n")
try:
    wdt = machine.WDT(id=0, timeout=6001)
except Exception:
    print("FAIL")

# valid test cases

print("\n***** Test 4: WDT created successfully 6s *****\n")
wdt = machine.WDT(id=0, timeout=500)
print(wdt)
print("PASS")

print("\n***** Test 5: WDT feed after 500ms *****\n")
time.sleep_ms(100)
wdt.feed()
print(wdt)
print("PASS")

print("\n***** Test 6: WDT feed after 900ms *****\n")
time.sleep_ms(200)
wdt.feed()
print(wdt)
print("PASS")

# reinitializing again fails

print("\n***** Test 7: trying to create WDT 2nd instance *****\n")
try:
    wdt = machine.WDT(0, timeout=1000)
except Exception:
    print("FAIL")"""


import machine
import time

# invalid test cases
print("\nTest 1: Wrong WDT id executing...")
try:
    wdt = machine.WDT(1, timeout=6000)
    print("Test 1 failed")
except Exception:
    pass

print("Test 2: WDT timeout too low executing...")
try:
    wdt = machine.WDT(0, 0)
    print("Test 2 failed")
except Exception:
    pass

print("Test 3: WDT timeout too high executing...")
try:
    wdt = machine.WDT(id=0, timeout=6001)
    print("Test 3 failed")
except Exception:
    pass

# valid test cases

print("Test 4: Rest cause check executing...")
try:
    wdt = machine.WDT(id=0, timeout=500)
except Exception:
    print("WDT instance creation failed!")

time.sleep_ms(500)
print("Sleep over")
