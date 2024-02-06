import time
from machine import Timer

t = Timer(0)
t.init(period=2000, mode=Timer.ONE_SHOT, callback=lambda t: print("Oneshot Timer"))
time.sleep(3)
t.deinit()

# TODO the whole time functionality is not really working...
# def blocking_delay_ms(delay_ms):
#     start = time.ticks_ms()
#     while time.ticks_diff(time.ticks_ms(), start) < delay_ms:
#         pass

# t1 = Timer(0)
# t1.init(period=2000, mode=Timer.PERIODIC, callback=lambda t: print("Periodic Timer"))
# blocking_delay_ms(2000)
# t.deinit()
