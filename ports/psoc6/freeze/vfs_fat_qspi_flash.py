import os
import machine, psoc6


# Try to mount the filesystem, and format the flash if it doesn't exist.
bdev = psoc6.QSPI_Flash()

try:
    vfs = os.VfsFat(bdev)
    os.mount(vfs, "/qspi_flash")
except:
    os.VfsFat.mkfs(bdev)
    vfs = os.VfsFat(bdev)
    os.mount(vfs, "/qspi_flash")

print("FAT filesystem mounted at /qspi_flash\n")

del machine, os, psoc6, bdev, vfs
