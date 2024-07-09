import machine
import os
import errno
import vfs

# Define constants
MOUNT_POINT = "/sd"
SHORT_TEST_STRING = "This is a test string."
LONG_TEST_STRING = "This is a very long string. And as a long string that it is, it is only getting longer and longer and the string goes. How long shall it be? Well, not really sure, but let´s try it like this."
READ_SIZE = 512
WRITE_SIZE = 512

board = os.uname().machine
if "CY8CPROTO-062-4343W" in board:
    sdcard_config = {
        "slot": 0,
        "width": 4,
        "cd": "P13_5",
        "cmd": "P12_4",
        "clk": "P12_5",
        "dat0": "P13_0",
        "dat1": "P13_1",
        "dat2": "P13_2",
        "dat3": "P13_3",
    }
elif "CY8CPROTO-063-BLE" in board:
    print("SKIP")
    raise SystemExit
elif "CY8CKIT-062S2-AI" in board:
    print("SKIP")
    raise SystemExit


def unmount_sd_card(path):
    try:
        os.umount(path)
    except OSError as e:
        if e.args[0] != errno.EINVAL:
            raise Exception(f"Could not unmount {path}")


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


def test_lfs2_file_transfer():
    bdev = machine.SDCard(**sdcard_config)

    # Unmount the SD card if mounted
    unmount_sd_card(MOUNT_POINT)

    # Mount or format the SD card with LFS2 filesystem
    if "VfsLfs2" in dir(os):
        mount_or_format_sd_card(bdev, os.VfsLfs2, MOUNT_POINT)

        print("\n***** Test 1: Short string file transfer to SD Card in LFS2 format *****\n")
        # Test short string
        short_test_file = MOUNT_POINT + "/test_lfs2_short.txt"
        if read_write_test(short_test_file, SHORT_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

        print("\n***** Test 2: Long string file transfer to SD Card in LFS2 format *****\n")
        # Test long string
        long_test_file = MOUNT_POINT + "/test_lfs2_long.txt"
        if read_write_test(long_test_file, LONG_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

    bdev.deinit()


def test_fat_file_transfer():
    bdev = machine.SDCard(**sdcard_config)

    # Unmount the SD card if mounted
    unmount_sd_card(MOUNT_POINT)

    # Mount or format the SD card with LFS2 filesystem
    if "VfsFat" in dir(os):
        mount_or_format_sd_card(bdev, os.VfsFat, MOUNT_POINT)

        print("\n***** Test 3: Short string file transfer to SD Card in FAT format *****\n")
        # Test short string
        short_test_file = MOUNT_POINT + "/test_fat_short.txt"
        if read_write_test(short_test_file, SHORT_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

        print("\n***** Test 4: Long string file transfer to SD Card in FAT format *****\n")
        # Test long string
        long_test_file = MOUNT_POINT + "/test_fat_long.txt"
        if read_write_test(long_test_file, LONG_TEST_STRING):
            print("PASS")
        else:
            print("FAIL")

    bdev.deinit()


def test_reintializing_same_slot():
    print("\n***** Test 3: reinitialize the same slot more than once *****\n")
    bdev1 = machine.SDCard(**sdcard_config)
    bdev1.deinit()
    bdev2 = machine.SDCard(**sdcard_config)
    bdev2.deinit()
    print("PASS")


if __name__ == "__main__":
    # TODO: Run the test based on the enabled file system
    test_fat_file_transfer()
    test_reintializing_same_slot()
