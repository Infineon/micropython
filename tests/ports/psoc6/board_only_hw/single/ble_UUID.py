import bluetooth


# Test cases for 16-bit, 32-bit, and 128-bit UUIDs
def test_uuids():
    print("=== UUID Testing ===\n")

    # Valid UUIDs
    try:
        print("Testing valid 16-bit UUID:")
        val16 = 0x180D  # Valid 16-bit UUID
        uuid16 = bluetooth.UUID(val16)
        print("Valid 16-bit UUID:", uuid16, "\n")
    except Exception as e:
        print("Error with valid 16-bit UUID:", e, "\n")

    try:
        print("Testing valid 32-bit UUID:")
        val32 = b"\x12\x34\x56\x78"  # Valid 32-bit UUID
        uuid32 = bluetooth.UUID(val32)
        print("Valid 32-bit UUID:", uuid32, "\n")
    except Exception as e:
        print("Error with valid 32-bit UUID:", e, "\n")

    try:
        print("Testing valid 128-bit UUID:")
        val128 = b"\x12\x34\x56\x78\x9a\xbc\xde\xf0\x12\x34\x56\x78\x9a\xbc\xde\xf0"  # Valid 128-bit UUID
        uuid128 = bluetooth.UUID(val128)
        print("Valid 128-bit UUID:", uuid128, "\n")
    except Exception as e:
        print("Error with valid 128-bit UUID:", e, "\n")

    # Invalid UUIDs
    try:
        print("Testing invalid 16-bit UUID:")
        val16_invalid = 0x1FFFF  # Invalid 16-bit UUID (too large)
        uuid16_invalid = bluetooth.UUID(val16_invalid)
        print("Invalid 16-bit UUID should not succeed:", uuid16_invalid, "\n")
    except Exception as e:
        print("Caught error with invalid 16-bit UUID:", e, "\n")

    try:
        print("Testing invalid 32-bit UUID:")
        val32_invalid = b"\x12\x34\x56\x56\x78"  # Invalid 32-bit UUID (not 4 bytes)
        uuid32_invalid = bluetooth.UUID(val32_invalid)
        print("Invalid 32-bit UUID should not succeed:", uuid32_invalid, "\n")
    except Exception as e:
        print("Caught error with invalid 32-bit UUID:", e, "\n")

    try:
        print("Testing invalid 128-bit UUID:")
        val128_invalid = b"\x12\x34\x56\x78\x12\x34\x56\x78"  # Invalid 128-bit UUID (not 16 bytes)
        uuid128_invalid = bluetooth.UUID(val128_invalid)
        print("Invalid 128-bit UUID should not succeed:", uuid128_invalid, "\n")
    except Exception as e:
        print("Caught error with invalid 128-bit UUID:", e, "\n")

    print("=== UUID Testing Complete ===")


# Run the test cases
test_uuids()
