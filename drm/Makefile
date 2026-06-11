EL8 := $(shell cat /etc/redhat-release 2>/dev/null | grep -c " 8." )
ifneq (,$(findstring 1, $(EL8)))
export EL8FLAG := -DEL8
endif

Raspbian := $(shell grep -Eic 'raspb(erry|ian)' /proc/cpuinfo /etc/os-release 2>/dev/null )
ifeq (,$(findstring 0, $(Raspbian)))
export RPIFLAG := -DRPI
endif

# Nota: la rama "ifneq ($(KERNELRELEASE),)" original era vestigial (el kernel
# lee drm/Kbuild, que tiene precedencia) y rompia el build bajo DKMS.

default: module

module:
	$(MAKE) -C $(KDIR) M=$(CURDIR)

clean:
	rm -rf *.mod *.mod.c *.o *.ko .tmp* .*.o.d .*.*.cmd Module.symvers modules.order

install:
	sudo cp usbdisp_drm.ko usbdisp_usb.ko /lib/modules/$(KVER)/kernel/drivers/usbdisp/

.PHONY: default module clean install
