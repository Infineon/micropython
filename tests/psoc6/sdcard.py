import machine

# invalid test cases
print("\n***** Test 1: Mount SD Card *****\n")

sdcard = machine.SDCARD(
    slot=1,
    width=4,
    cd="P13_5",
    cmd="P12_4",
    clk="P12_5",
    dat0="P13_0",
    dat1="P13_1",
    dat2="P13_2",
    dat3="P13_3",
)

print("\n***** initialized SD card = *****\n")
print(sdcard)
