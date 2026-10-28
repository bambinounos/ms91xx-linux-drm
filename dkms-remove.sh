#!/bin/bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root or with sudo:"
    echo "  sudo ./dkms-remove.sh"
    exit 1
fi

KVER="${1:-$(uname -r)}"

echo "=== Uninstalling msdisp driver from DKMS ==="
for v in $(dkms status msdisp 2>/dev/null | awk -F'[,/]' '{print $2}' | sort -u); do
    echo "Removing msdisp version $v from DKMS..."
    dkms remove "msdisp/$v" --all 2>/dev/null || true
    rm -rf "/usr/src/msdisp-$v"
done

if [ -d "/lib/modules/$KVER" ]; then
    while IFS= read -r -d '' stale_ko; do
        echo "Removing leftover module file: $stale_ko"
        rm -f "$stale_ko"
    done < <(find "/lib/modules/$KVER" \( -name "usbdisp_drm.ko*" -o -name "usbdisp_usb.ko*" \) -print0 2>/dev/null)
    depmod -a "$KVER"
fi

if command -v update-initramfs >/dev/null 2>&1; then
    echo "Updating initramfs for kernel $KVER..."
    update-initramfs -u -k "$KVER" || true
elif command -v dracut >/dev/null 2>&1; then
    echo "Updating initramfs (dracut) for kernel $KVER..."
    dracut -f --kver "$KVER" || true
elif command -v mkinitcpio >/dev/null 2>&1; then
    echo "Updating initramfs (mkinitcpio)..."
    mkinitcpio -P || true
fi

echo "Unloading kernel modules (if not in use)..."
modprobe -r usbdisp_usb usbdisp_drm 2>/dev/null || true

echo "=== msdisp driver completely uninstalled (reboot if modules remain in memory) ==="
