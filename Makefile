export HAL_PATH := $(CURDIR)/usb_hal
export DRM_PATH := $(CURDIR)/drm
export USB_HAL := hal_adaptor.o usb_device.o usb_hal_interface.o usb_hal_sysfs.o usb_hal_thread.o

# Nota: la rama "ifneq ($(KERNELRELEASE),) include Kbuild" original era vestigial
# (el kernel lee drm/Kbuild directamente) y rompia el build bajo DKMS, que pasa
# KERNELRELEASE=<ver> en la linea de comandos.

ifeq ($(KVER),)
	KVER := $(shell uname -r)
endif

ifeq ($(KDIR),)
	KDIR := /lib/modules/$(KVER)/build
endif

export KVER KDIR

default: drm FORCE

drm: FORCE
	@echo "drm build"
	$(MAKE) -C $(DRM_PATH)

clean: FORCE
	$(MAKE) -C $(HAL_PATH) clean
	$(MAKE) -C $(DRM_PATH) clean

FORCE:

.PHONY: default drm clean FORCE
