EL8 := $(shell cat /etc/redhat-release 2>/dev/null | grep -c " 8." )
ifneq (,$(findstring 1, $(EL8)))
export EL8FLAG := -DEL8
endif

Raspbian := $(shell grep -Eic 'raspb(erry|ian)' /proc/cpuinfo /etc/os-release 2>/dev/null )
ifeq (,$(findstring 0, $(Raspbian)))
export RPIFLAG := -DRPI
endif


ifneq ($(KERNELRELEASE),)
ccflags-y := -isystem include/drm $(CFLAGS) $(EL8FLAG) $(RPIFLAG)
ccflags-usbdisp_usb := -I$(HAL_PATH)
usbdisp_drm-y := msdisp_plat_drv.o msdisp_plat_dev.o msdisp_drm_drv.o  msdisp_drm_modeset.o  msdisp_drm_gem.o  \
	msdisp_drm_fb.o  msdisp_drm_encoder.o  msdisp_drm_connector.o msdisp_drm_interface.o msdisp_drm_sysfs.o msdisp_common_util.o
usbdisp_usb-y := msdisp_usb_drv.o ms9132_hal.o $(USB_HAL)

else

# kbuild against specified or current kernel
#ifeq ($(KVER),)
#	KVER := $(shell uname -r)
#endif

#ifeq ($(KDIR),)
#	KDIR := /lib/modules/$(KVER)/build
#endif

default: module

module:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	rm -rf *.mod *.mod.c *.o *.ko .tmp* .*.o.d .*.*.cmd Module.symvers modules.order

install:
	sudo cp usbdisp_drm.ko usbdisp_usb.ko /lib/modules/$(KVER)/kernel/drivers/usbdisp/

endif
