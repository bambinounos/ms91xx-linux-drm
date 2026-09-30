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
# Unbind platform devices first so DRM device is unplugged and userspace releases it
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
else
    echo "=== AVISO: El módulo usbdisp_drm sigue en uso por la sesión gráfica actual (versión cargada: $LOADED_VER). ==="
    echo "=== Para que tome efecto el soporte de cursor (v$VERSION), es necesario CERRAR SESIÓN y volver a entrar (o REINICIAR el equipo). ==="
fi
