# Makefile for my-hello-module kernel module
#
# This Makefile supports:
# 1. Direct kernel module build:  make
# 2. DKMS build (used by the Debian package)
#
# When building via DKMS, the kernel build system provides KERNELDIR,
# so this Makefile works for both approaches.

obj-m += myhello.o

myhello-objs := src/hello.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	depmod -a

.PHONY: all clean install