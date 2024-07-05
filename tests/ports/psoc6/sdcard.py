import machine
import os
import errno
import vfs

# Define constants
MOUNT_POINT_LFS2 = "/SDCardLFS2"
MOUNT_POINT_FAT = "/SDCardFAT"
SHORT_TEST_STRING = "This is a test string."
LONG_TEST_STRING = "This is a very long string. And as a long string that it is, it is only getting longer and longer and the string goes. How long shall it be? Well, not really sure, but let´s try it like this."
READ_SIZE = 512
WRITE_SIZE = 512

print("\n***** Test 1: File transfer to SD Card in LFS2 farmat *****\n")
print(machine_name)

test_string = "This is a test string."
long_test_string = "This is a very long string. And as a long string that it is, it is only getting longer and longer and the string goes. How long shall it be? Well, not really sure, but let´s try it like this."

# first priority is always LFS2 filesystem as it is the default
if "VfsLfs2" in dir(os):
    # open a file and do some operation
    print("write to /sd/test_sd_lfs2_short.txt")
    f = open("/sd/test_sd_lfs2_short.txt", "w")
    f.write(test_string)
    f.close()

def mount_or_format_sd_card(block_device, filesystem, mount_point):
    try:
        if filesystem == os.VfsLfs2:
            vfs = filesystem(block_device, progsize=WRITE_SIZE, readsize=READ_SIZE)
        else:
            vfs = filesystem(block_device)
        os.mount(vfs, mount_point)
    except OSError:
        if filesystem == os.VfsLfs2:
            filesystem.mkfs(block_device, progsize=WRITE_SIZE, readsize=READ_SIZE)
            vfs = filesystem(block_device, progsize=WRITE_SIZE, readsize=READ_SIZE)
        else:
            filesystem.mkfs(block_device)
            vfs = filesystem(block_device)
        os.mount(vfs, mount_point)
    print(f"\nSD card mounted at {mount_point}\n")


def read_write_test(file_path, test_data):
    with open(file_path, "w") as f:
        f.write(test_data)
    with open(file_path, "r") as f:
        return f.read() == test_data


def sd_card_instance():
    # Define the SD card configuration
    sdcard_config = {
        "slot": 1,
        "width": 4,
        "cd": "P13_5",
        "cmd": "P12_4",
        "clk": "P12_5",
        "dat0": "P13_0",
        "dat1": "P13_1",
        "dat2": "P13_2",
        "dat3": "P13_3",
    }
    return machine.SDCard(**sdcard_config)


def test_lfs2_file_transfer():
    bdev = sd_card_instance()

    # Unmount the SD card if mounted
    unmount_sd_card(MOUNT_POINT_LFS2)

    # Mount or format the SD card with LFS2 filesystem
    if "VfsLfs2" in dir(os):
        mount_or_format_sd_card(bdev, os.VfsLfs2, MOUNT_POINT_LFS2)

        print("\n***** Test 1: Short string file transfer to SD Card in LFS2 format *****\n")
        # Test short string
        short_test_file = MOUNT_POINT_LFS2 + "/test_sd_lfs2_short.txt"
        if read_write_test(short_test_file, SHORT_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

        print("\n***** Test 2: Long string file transfer to SD Card in LFS2 format *****\n")
        # Test long string
        long_test_file = MOUNT_POINT_LFS2 + "/test_sd_lfs2_long.txt"
        if read_write_test(long_test_file, LONG_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")


def test_fat_file_transfer():
    bdev = sd_card_instance()

    # Unmount the SD card if mounted
    unmount_sd_card(MOUNT_POINT_FAT)

    # Mount or format the SD card with LFS2 filesystem
    if "VfsLfs2" in dir(os):
        mount_or_format_sd_card(bdev, os.VfsFat, MOUNT_POINT_FAT)

        print("\n***** Test 3: Short string file transfer to SD Card in FAT format *****\n")
        # Test short string
        short_test_file = MOUNT_POINT_FAT + "/test_sd_fat_short.txt"
        if read_write_test(short_test_file, SHORT_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

        print("\n***** Test 4: Long string file transfer to SD Card in FAT format *****\n")
        # Test long string
        long_test_file = MOUNT_POINT_FAT + "/test_sd_fat_long.txt"
        if read_write_test(long_test_file, LONG_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")


def test_reintializing_same_slot():
    print("\n***** Test 5: reinitialize the same slot more than once *****\n")

    bdev1 = sd_card_instance()
    bdev2 = sd_card_instance()
    bdev3 = sd_card_instance()
    print("PASS")


def test_negative_slot_number():
    print("\n***** Test 6: slot number exceeding the number of available slots *****\n")

    try:
        sdcard_config = {
            "slot": 2,
            "width": 4,
            "cd": "P13_5",
            "cmd": "P12_4",
            "clk": "P12_5",
            "dat0": "P13_0",
            "dat1": "P13_1",
            "dat2": "P13_2",
            "dat3": "P13_3",
        }
        bdev = machine.SDCard(**sdcard_config)
    except Exception:
        print("FAIL")


if __name__ == "__main__":
    test_lfs2_file_transfer()
    test_fat_file_transfer()
    test_reintializing_same_slot()
    test_negative_slot_number()
