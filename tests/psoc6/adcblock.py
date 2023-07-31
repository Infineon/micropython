"""
ADCBlock test for the PSoC6 port.
"""
from machine import Pin, ADCBlock

print("ADCBlock tests")

pin = Pin("P10_0")

# Negative tests
try:
    adcBlock = ADCBlock(1)
except:
    print("TypeError: Specified ADC id not supported. Currently only block 0 is configured!")

try:
    adcBlock = ADCBlock(0, bits=10)
except:
    print("TypeError: Invalid bits. Current ADC configuration supports only 12 bits resolution!")

adcBlock = ADCBlock(0)

try:
    adcPin = adcBlock.connect(1, pin)
except:
    print("TypeError: Wrong pin specified for the mentioned channel")


adcPin = adcBlock.connect(0, pin)

print(adcPin.read_uv() > 0)
