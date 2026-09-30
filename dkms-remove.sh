#!/bin/bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root or with sudo:"
    echo "  sudo ./dkms-remove.sh"
    exit 1
fi

echo "=== Uninstalling msdisp driver from DKMS ==="
for v in $(dkms status msdisp 2>/dev/null | awk -F'[,/]' '{print $2}' | sort -u); do
    echo "Removing msdisp version $v from DKMS..."
    dkms remove msdisp/$v --all 2>/dev/null || true
    rm -rf "/usr/src/msdisp-$v"
done

echo "Unloading kernel modules..."
modprobe -r usbdisp_usb usbdisp_drm 2>/dev/null || true

echo "=== msdisp driver completely uninstalled ==="
