# kernel-module — Packaging a Linux Kernel Module with APT

This directory contains a sample Debian package (`my-hello-module`) that demonstrates how to bundle a Linux kernel module for installation via APT, using DKMS (Dynamic Kernel Module Support) for automatic rebuilds across kernel updates.

## Package: `my-hello-module`

A simple kernel module that creates `/proc/myhello` with a greeting message. It demonstrates the complete lifecycle of an APT-packaged kernel module: build, install, load, unload, and remove — all with DKMS integration.

### Verified Build & Install

```bash
cd kernel-module/my-hello-module

# Install build dependencies
sudo apt install debhelper fakeroot dkms linux-headers-generic
sudo apt build-dep ./

# Build the binary package
dpkg-buildpackage -us -uc -b

# Install
sudo apt install ../my-hello-module_*.deb

# Verify DKMS built and loaded the module
dkms status
lsmod | grep myhello
ls -la /usr/src/my-hello-module-1.0.0/

# Remove
sudo apt remove my-hello-module
```

### Generated Artifacts

When you run `dpkg-buildpackage`, the following files are generated in the parent directory:

**Binary Package:**
- `my-hello-module_1.0.0-1_amd64.deb` — The installable Debian package containing module source and DKMS configuration

**Build Metadata:**
- `my-hello-module_1.0.0-1.dsc` — Source package descriptor (lists dependencies, including `dkms`, `linux-headers`)
- `my-hello-module_1.0.0.orig.tar.gz` — Original source archive
- `my-hello-module_1.0.0-1.debian.tar.xz` — Debian-specific patches and configuration (from `debian/` directory)
- `my-hello-module_1.0.0-1_amd64.buildinfo` — Build environment information
- `my-hello-module_1.0.0-1_amd64.changes` — Summary of changes and package contents

**Inside the `.deb` Package (visible after `dpkg -X`):**
- `usr/src/my-hello-module-1.0.0/` — Module source directory (registered with DKMS)
- `usr/src/my-hello-module-1.0.0/src/hello.c` — Kernel module source code
- `usr/src/my-hello-module-1.0.0/Makefile` — Kbuild-compatible build file
- `usr/src/my-hello-module-1.0.0/dkms.conf` — DKMS configuration for automatic rebuilds
- `usr/share/doc/my-hello-module/` — Documentation files (changelog, copyright)
- `DEBIAN/control` — Package metadata with `dkms` and `linux-image-generic` dependencies
- `DEBIAN/postinst` — Post-installation script (runs `dkms add/build/install`)
- `DEBIAN/prerm` — Pre-removal script (runs `dkms remove`)

**After Installation (via DKMS):**
- `lib/modules/<kernel-version>/extra/myhello.ko` — Compiled kernel module (built by DKMS for current kernel)
- Additional `.ko` files for any new kernels installed after the package

### What This Demonstrates

| Concept | Implementation |
|---------|---------------|
| **DKMS integration** | `dkms.conf` registers the module for automatic rebuild on kernel updates |
| **Source installation** | Module source is installed to `/usr/src/my-hello-module-1.0.0/` |
| **Auto-load on install** | `debian/postinst` runs `dkms add/build/install` then `modprobe` |
| **Auto-unload on remove** | `debian/prerm` runs `modprobe -r` then `dkms remove` |
| **Kernel version compatibility** | Module is compiled against the running kernel's headers |
| **procfs interface** | Module creates `/proc/myhello` for user-space interaction |

### Debian Packaging Files

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata with `dkms` and `linux-image-generic` dependencies |
| `debian/rules` | Installs source to `/usr/src/<pkg>-<ver>/` for DKMS |
| `debian/changelog` | **Required** — version history (was missing, added) |
| `debian/compat` | Debhelper compatibility level 13 (was missing, added) |
| `debian/source/format` | Source format `3.0 (quilt)` |
| `debian/postinst` | Registers with DKMS, builds, installs, and loads the module |
| `debian/prerm` | Unloads the module and removes it from DKMS |
| `dkms.conf` | DKMS configuration: package name, version, build commands |
| `Makefile` | Kernel module build system (works with both direct and DKMS builds) |

> **Note:** The `debian/rules` `override_dh_auto_clean` uses a simple `rm` instead of invoking the kernel build system clean, because the kernel headers may not match the running kernel during package build on a different machine.

### How DKMS Works

DKMS (Dynamic Kernel Module Support) solves the problem of kernel module compatibility across kernel updates:

1. **Package install**: `postinst` runs `dkms add` to register the module source, then `dkms build` to compile it against the current kernel, then `dkms install` to copy the `.ko` to the kernel's module tree.

2. **Kernel update**: When a new kernel is installed (e.g., via `apt upgrade`), DKMS automatically rebuilds all registered modules for the new kernel. This happens via the kernel post-install hooks.

3. **Module lifecycle**: The module `.ko` file lives in `/lib/modules/<kernel-version>/updates/dkms/myhello.ko`. The `depmod` command updates module dependencies so `modprobe` can find it.

### Build Pipeline

```
src/hello.c  ──┐
Makefile     ──┤──► make -C /lib/modules/$(uname -r)/build M=$PWD
dkms.conf    ──┘
                    │
                    ▼
              myhello.ko  (kernel module binary)
                    │
                    ▼
              sudo insmod myhello.ko
                    │
                    ▼
              /proc/myhello  (user-space interface)
```

### Module Source Breakdown

```c
// Module entry/exit points
module_init(hello_init);    // Called on insmod / modprobe
module_exit(hello_exit);    // Called on rmmod

// /proc interface
proc_create("myhello", ...) // Creates /proc/myhello on init
remove_proc_entry(...)      // Removes it on exit

// Metadata embedded in the .ko
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("Sample kernel module packaged via APT with DKMS support");
```

### Maintainer Script Lifecycle

**Install:**
```
postinst configure →
  1. dkms add -m my-hello-module -v 1.0.0
  2. dkms build -m my-hello-module -v 1.0.0
  3. dkms install -m my-hello-module -v 1.0.0
  4. modprobe myhello
```

**Remove:**
```
prerm remove →
  1. modprobe -r myhello
  2. dkms remove -m my-hello-module -v 1.0.0 --all
```

### DKMS Configuration

```conf
PACKAGE_NAME="my-hello-module"
PACKAGE_VERSION="1.0.0"

# Build command (${kernelver} is substituted by DKMS)
MAKE[0]="make -C /lib/modules/${kernelver}/build M=... modules"

# Module binary name and install location
BUILT_MODULE_NAME[0]="myhello"
DEST_MODULE_LOCATION[0]="/extra"

# Auto-rebuild on kernel update
AUTOINSTALL="yes"
```
