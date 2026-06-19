# HowToAptPackage

A comprehensive sample project demonstrating how to create Debian/APT packages for three types of system-level components:

1. **systemd-service/** — A systemd-managed daemon (user-space binary + systemd unit)
2. **ebpf-program/** — A CO-RE (Compile Once - Run Everywhere) eBPF program
3. **kernel-module/** — A Linux kernel module with DKMS support

Each sub-project is a fully structured Debian source package that can be built with `dpkg-buildpackage` and installed/removed with `apt`.

---

## Debian Packaging Concepts

### What is a Debian Package?

A Debian package (`.deb`) is an archive that contains:
- **Binary files** (executables, libraries, kernel modules, etc.)
- **Configuration files**
- **Maintainer scripts** (`preinst`, `postinst`, `prerm`, `postrm`) that run at install/remove time
- **Metadata** (package name, version, dependencies, description)

### Key Files in a Debian Source Package

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata: name, version, dependencies, description |
| `debian/rules` | Build script (Makefile) — tells `dpkg-buildpackage` how to compile and install |
| `debian/compat` | Debhelper compatibility level (currently 13) |
| `debian/source/format` | Source package format (`3.0 (quilt)` is the modern standard) |
| `debian/install` | Lists files to install and their target paths |
| `debian/*.service` | systemd service unit files (auto-detected by dh_installinit) |
| `debian/postinst` | Post-installation script (e.g., start a service, load a module) |
| `debian/prerm` | Pre-removal script (e.g., stop a service, unload a module) |

### The Build Process

```
dpkg-buildpackage -us -uc
```

This runs `debian/rules` through debhelper, which:
1. **clean** — Removes previous build artifacts
2. **build** — Compiles the source code
3. **install** — Copies files into a temporary directory (`debian/<package>/`)
4. **binary** — Creates the `.deb` package

### Maintainer Script Lifecycle

```
Install:   preinst → (files copied) → postinst
Remove:    prerm   → (files removed) → postrm
Upgrade:   preinst (old) → prerm (old) → postinst (new)
```

### Package Dependencies

- **Depends**: Hard requirement — package won't install without these
- **Recommends**: Strong suggestion — installed by default but can be omitted
- **Suggests**: Optional — user is informed but not required
- **Build-Depends**: Required to compile the source package

### Building Packages

```bash
# Install build dependencies
sudo apt build-dep ./

# Build the package
dpkg-buildpackage -us -uc -b

# Install the resulting .deb
sudo apt install ../<package>.deb

# Remove the package
sudo apt remove <package>
```

---

## Project Structure

```
HowToAptPackage/
├── README.md                    # This file — general Debian concepts
├── systemd-service/
│   └── my-daemon/               # systemd service package
│       ├── src/main.c           # Daemon source code
│       └── debian/              # Debian packaging files
├── ebpf-program/
│   └── my-bpf-sensor/           # CO-RE eBPF package
│       ├── src/                 # BPF program + user-space loader
│       ├── Makefile             # Build system for BPF + loader
│       └── debian/              # Debian packaging files
└── kernel-module/
    └── my-hello-module/         # Kernel module package
        ├── src/hello.c          # Module source code
        ├── Makefile             # Kernel module build system
        ├── dkms.conf            # DKMS configuration
        └── debian/              # Debian packaging files
```

---

## Quick Start

```bash
# 1. Build the systemd service package
cd systemd-service/my-daemon
dpkg-buildpackage -us -uc -b
sudo apt install ../my-daemon_*.deb

# 2. Build the eBPF sensor package
cd ../../ebpf-program/my-bpf-sensor
dpkg-buildpackage -us -uc -b
sudo apt install ../my-bpf-sensor_*.deb

# 3. Build the kernel module package
cd ../../kernel-module/my-hello-module
dpkg-buildpackage -us -uc -b
sudo apt install ../my-hello-module_*.deb
```

See each sub-directory's README for detailed instructions.