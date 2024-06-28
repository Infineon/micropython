### I2C
from machine import UART
import machine
import binascii
import time
import os

# Allocate pin based on board
machine = os.uname().machine
if "CY8CPROTO-062-4343W" in machine:
    uart_rx_pin = "P6_0"
    uart_tx_pin = "P6_1"
    uart_rts_pin = "P6_2"
    uart_cts_pin = "P6_3"
elif "CY8CPROTO-063-BLE" in machine:
    uart_rx_pin = "P6_0"
    uart_tx_pin = "P6_1"
    uart_rts_pin = "P6_2"
    uart_cts_pin = "P6_3"

# 1. Construct instance
##############################################
uart = UART(1)
uart.init(9600, bits=8, parity=None, stop=1, tx=uart_tx_pin, rx=uart_rx_pin, rxbuf=8)


def uart_tests():
    # 2. Basic tests
    ##############################################

    # write_char()
    # read_char()
    uart.writechar(1)
    rx_char = uart.readchar()
    print("Tx char is received by Rx char: ", rx_char == 1)

    # read()
    tx_data = b"abcdefg"
    uart.write(tx_data)
    time.sleep_ms(100)
    rx_data = uart.read()
    print("Tx is received by Rx(read()): ", rx_data == tx_data)

    # readline(nbytes)
    tx_data = "abcd\ne"
    uart.write(tx_data)
    time.sleep_ms(100)
    rx_data = uart.readline()
    print(rx_data)
    print("Tx is received by Rx(readline()): ", rx_data == b"abcd\n")
    uart.read()  # read all data available to clear buffer

    # read()
    tx_data = "abcd\nefg\n"
    uart.write(tx_data)
    time.sleep_ms(100)
    rx_data = uart.read(10)
    print(rx_data)
    print("Tx is received by Rx(read(nbytes)):", rx_data == b"efg\nabcd\nefg\n")

    # tx_done()
    tx_data = "abcdefg"
    uart.write(tx_data)
    print("Tx Ongoing: ", uart.txdone() == False)
    time.sleep_ms(100)
    print("Tx Done: ", uart.txdone() == True)
    uart.read()  # read all data available to clear buffer

    # write(buf)
    # readinto(buf)
    uart_rx_buf = bytearray(8)
    tx_data = b"\x01\x44\x17\x88\x98\x11\x34\xff"
    uart.write(tx_data)
    time.sleep_ms(100)
    uart.readinto(uart_rx_buf)
    print("Tx is received by Rx(readinto(buf)): ", uart_rx_buf == tx_data)


def uart_interrupt_tests():
    uart.irq(trigger=UART.TX_DONE, handler=tx_complete)
    tx_data = b"Hi this is data"
    uart.write(tx_data)
    time.sleep_ms(100)
    while not tx_done:
        pass
    print("Tx Done")

    uart.irq(trigger=UART.RX_DONE, handler=rx_complete)
    uart_rx_buf = bytearray(8)
    uart.readinto(uart_rx_buf)
    while not rx_done:
        pass
    print("Rx Done")

    uart.irq(trigger=UART.RX_FULL, handler=rx_full)
    tx_data = b"\x44\x17\x88\x98\x11\x34\xff\x12\x57\x76\x44\x17\x88\x98\x11\x34\xff\x12\x57\x76\x44\x17\x88\x98\x11\x34\xff\x12\x57\x76\x44\x17\x88\x98\x11\x34\xff\x12\x57\x76"
    uart.write(tx_data)
    uart.read()
    while not rx_full:
        pass
    print("Rx Buffer Full")

    uart.irq(trigger=UART.TX_EMPTY, handler=tx_empty)
    while not tx_empty:
        pass
    print("Tx Empty")


print("********UART Tests********\n")
uart_tests()

tx_done = False


def tx_complete(t):
    global tx_done
    tx_done = True


rx_done = False


def rx_complete(t):
    global rx_done
    rx_done = True


rx_full = False


def rx_full(t):
    global rx_full
    rx_full = True


tx_empty = False


def tx_empty(t):
    global tx_empty
    tx_empty = True


print("\n*******UART Interrupt Tests*******\n")
uart_interrupt_tests()
uart.deinit()
