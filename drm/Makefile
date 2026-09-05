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

# kbuild no admite compilar un arbol en una ruta con espacios: copia M= a
# KBUILD_EXTMOD y lo rechaza con "$(if $(word 2,$(KBUILD_EXTMOD)),...)", o sea
# parte la ruta por espacios dentro de make, despues de que el shell la paso como
# un solo argumento. Aborta con "building multiple external modules is not
# supported", que no explica la causa real, asi que fallamos aqui con un mensaje
# claro. DKMS no lo sufre: construye en /usr/src/<paquete>-<version>. Solo afecta
# a "make"; "make clean" usa rutas relativas y sigue funcionando.
ifneq ($(words $(CURDIR)),1)
BUILD_PATH_HAS_SPACES := $(CURDIR)
endif

default: module

module:
ifdef BUILD_PATH_HAS_SPACES
	$(error cannot build from a path with spaces: '$(BUILD_PATH_HAS_SPACES)'. kbuild splits M= on whitespace, so an in-place build is impossible. Copy the tree to a space-free path, or use DKMS -- it builds in /usr/src/)
else
	$(MAKE) -C "$(KDIR)" M="$(CURDIR)"
endif

clean:
	rm -rf *.mod *.mod.c *.o *.ko .tmp* .*.o.d .*.*.cmd Module.symvers modules.order

install:
	sudo cp usbdisp_drm.ko usbdisp_usb.ko /lib/modules/$(KVER)/kernel/drivers/usbdisp/

.PHONY: default module clean install
