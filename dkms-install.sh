#!/bin/bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root or with sudo:"
    echo "  sudo ./dkms-install.sh"
    exit 1
fi

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION=$(grep -m1 '^PACKAGE_VERSION=' "$DIR/dkms.conf" | cut -d'"' -f2)
SRC_DIR="/usr/src/msdisp-$VERSION"
KVER="${1:-$(uname -r)}"

echo "=== Installing msdisp driver version $VERSION for kernel $KVER ==="

# 1. Clean up any obsolete/stale versions of msdisp in DKMS
for v in $(dkms status msdisp 2>/dev/null | awk -F'[,/]' '{print $2}' | sort -u); do
    echo "Purging previous msdisp version $v from DKMS..."
    dkms remove "msdisp/$v" --all 2>/dev/null || true
    if [ "$v" != "$VERSION" ]; then
        rm -rf "/usr/src/msdisp-$v"
    fi
done

# 2. Remove any orphaned/manual .ko files in /lib/modules/$KVER that could shadow DKMS
if [ -d "/lib/modules/$KVER" ]; then
    while IFS= read -r -d '' stale_ko; do
        echo "Removing stale module file: $stale_ko"
        rm -f "$stale_ko"
    done < <(find "/lib/modules/$KVER" \( -name "usbdisp_drm.ko*" -o -name "usbdisp_usb.ko*" \) -print0 2>/dev/null)
fi

# 3. Sync source tree to /usr/src/msdisp-<version>
echo "Copying source tree to $SRC_DIR..."
mkdir -p "$SRC_DIR"
rsync -a --delete --exclude .git --exclude "*.o" --exclude "*.ko" --exclude "*.mod*" --exclude "*.cmd" --exclude ".*.cmd" --exclude "Module.symvers" --exclude "modules.order" "$DIR/" "$SRC_DIR/"

# 4. Add and install into DKMS
echo "Registering msdisp/$VERSION in DKMS..."
dkms add "msdisp/$VERSION"

echo "Building and signing msdisp/$VERSION for $KVER..."
dkms install "msdisp/$VERSION" -k "$KVER" --force
depmod -a "$KVER"

DISK_PATH=$(modinfo -k "$KVER" -n usbdisp_drm 2>/dev/null || echo "unknown")
DISK_VER=$(modinfo -k "$KVER" -F version usbdisp_drm 2>/dev/null || echo "unknown")
echo "Resolved on-disk module: $DISK_PATH (version: $DISK_VER)"

# 5. Rebuild initramfs so early boot (Dracut / initramfs-tools / hibernate resume)
# never loads a cached older usbdisp_drm.ko from /boot/initrd.img-$KVER
if command -v update-initramfs >/dev/null 2>&1; then
    echo "Updating initramfs for kernel $KVER (update-initramfs)..."
    update-initramfs -u -k "$KVER" || echo "Warning: update-initramfs returned non-zero, continuing..."
elif command -v dracut >/dev/null 2>&1; then
    echo "Updating initramfs for kernel $KVER (dracut)..."
    dracut -f --kver "$KVER" || echo "Warning: dracut returned non-zero, continuing..."
elif command -v mkinitcpio >/dev/null 2>&1; then
    echo "Updating initramfs (mkinitcpio)..."
    mkinitcpio -P || echo "Warning: mkinitcpio returned non-zero, continuing..."
fi

# 6. Reload kernel modules if not pinned by DRM/fbcon/compositor
echo "Attempting live module reload..."
for dev in /sys/bus/platform/drivers/msdisp_plat/msdisp_plat.*; do
    if [ -e "$dev" ]; then
        dev_name=$(basename "$dev")
        echo "Unbinding $dev_name..."
        echo "$dev_name" > /sys/bus/platform/drivers/msdisp_plat/unbind 2>/dev/null || true
    fi
done
sleep 1

modprobe -r usbdisp_usb 2>/dev/null || true
modprobe -r usbdisp_drm 2>/dev/null || true
modprobe usbdisp_drm 2>/dev/null || true
modprobe usbdisp_usb 2>/dev/null || true

LOADED_VER=$(cat /sys/module/usbdisp_drm/version 2>/dev/null || echo "not loaded")
if [ "$LOADED_VER" = "$VERSION" ]; then
    echo "=== msdisp $VERSION successfully loaded into running kernel! ==="
elif [ "$LOADED_VER" = "not loaded" ]; then
    echo "=== msdisp $VERSION installed for kernel $KVER (module will auto-load when the USB display is plugged in, or after reboot). ==="
else
    echo ""
    echo "================================================================================"
    echo "ACTION REQUIRED: FULL REBOOT NEEDED (sudo reboot)"
    echo "  Running in RAM right now : usbdisp_drm v$LOADED_VER (pinned by DRM/fbdev/session)"
    echo "  Installed on disk/initrd : usbdisp_drm v$DISK_VER"
    echo "  Logging out is NOT enough because kernel fbdev keeps the old module pinned."
    echo "  Please run 'sudo reboot' now, then verify with:"
    echo "    cat /sys/module/usbdisp_drm/version   # must output $VERSION"
    echo "--------------------------------------------------------------------------------"
    echo "ACCIÓN REQUERIDA: REINICIO COMPLETO NECESARIO (sudo reboot)"
    echo "  Versión activa en RAM    : usbdisp_drm v$LOADED_VER (retenida por DRM/fbdev)"
    echo "  Instalada en disco/initrd: usbdisp_drm v$DISK_VER"
    echo "  Cerrar sesión NO basta; ejecute 'sudo reboot' para cargar v$VERSION."
    echo "================================================================================"
fi
