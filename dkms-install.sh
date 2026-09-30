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

# Clean up any obsolete/stale versions of msdisp in DKMS
for v in $(dkms status msdisp 2>/dev/null | awk -F'[,/]' '{print $2}' | sort -u); do
    echo "Purging previous msdisp version $v from DKMS..."
    dkms remove msdisp/$v --all 2>/dev/null || true
    if [ "$v" != "$VERSION" ]; then
        rm -rf "/usr/src/msdisp-$v"
    fi
done

# Sync source tree to /usr/src/msdisp-<version>
echo "Copying source tree to $SRC_DIR..."
mkdir -p "$SRC_DIR"
rsync -a --delete --exclude .git --exclude "*.o" --exclude "*.ko" --exclude "*.mod*" --exclude "*.cmd" --exclude ".*.cmd" --exclude "Module.symvers" --exclude "modules.order" "$DIR/" "$SRC_DIR/"

# Add and install into DKMS
echo "Registering msdisp/$VERSION in DKMS..."
dkms add msdisp/$VERSION

echo "Building and signing msdisp/$VERSION for $KVER..."
dkms install msdisp/$VERSION -k "$KVER"

# Reload kernel modules if device is attached
echo "Reloading kernel modules..."
modprobe -r usbdisp_usb usbdisp_drm 2>/dev/null || true
modprobe usbdisp_usb 2>/dev/null || true

echo "=== msdisp $VERSION successfully installed for $KVER! ==="
