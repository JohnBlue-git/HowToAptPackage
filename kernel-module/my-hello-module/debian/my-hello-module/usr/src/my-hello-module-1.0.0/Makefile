# Makefile for my-hello-module kernel module
#
# This Makefile supports:
# 1. Direct kernel module build:  make
# 2. DKMS build (used by the Debian package)
#
# When building via DKMS, the kernel build system provides KERNELDIR,
# so this Makefile works for both approaches.
#
# Uses out-of-tree build in build/ to keep source tree clean.

SRCDIR := src
BUILDDIR := build

obj-m += $(BUILDDIR)/myhello.o

$(BUILDDIR)/myhello-objs := $(SRCDIR)/hello.o

all: $(BUILDDIR) modules

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	depmod -a

.PHONY: all clean install
