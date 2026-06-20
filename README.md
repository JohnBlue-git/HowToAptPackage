# HowToAptPackage

A comprehensive sample project demonstrating how to create Debian/APT packages for three types of system-level components:

1. **systemd-service/** — A systemd-managed daemon (user-space binary + systemd unit)
2. **ebpf-program/** — A CO-RE (Compile Once - Run Everywhere) eBPF program  
3. **kernel-module/** — A Linux kernel module with DKMS support

Each sub-project is a fully structured Debian source package that can be built with `dpkg-buildpackage` and installed/removed with `apt`. All three packages have been **verified with real build, install, and remove tests**.

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
| `debian/changelog` | **Required** — version history and release notes |
| `debian/compat` | Debhelper compatibility level (currently 13) |
| `debian/source/format` | Source package format (`3.0 (quilt)` is the modern standard) |
| `debian/*.service` | systemd service unit files (auto-detected by `dh_installsystemd`) |
| `debian/postinst` | Post-installation script (e.g., start a service, load a module) |
| `debian/prerm` | Pre-removal script (e.g., stop a service, unload a module) |

### The Build Process

```bash
dpkg-buildpackage -us -uc -b
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

### Common Pitfalls (Fixed in This Project)

| Issue | Solution |
|-------|----------|
| Missing `debian/changelog` | Required by `dpkg-buildpackage` — added to all packages |
| Missing `debian/compat` | Required by debhelper — added to eBPF and kernel packages |
| Double compat declaration | Use either `debian/compat` file OR `debhelper-compat` in `Build-Depends`, not both |
| systemd service in both `lib/` and `usr/lib/` | Don't use `debian/install`; let `dh_installsystemd` handle it |
| `lib -> usr/lib` symlink conflict | On merged-usr systems, only install to `/usr/lib/systemd/system/` |

---

## Project Structure

```
HowToAptPackage/
├── README.md                    # This file — general Debian concepts
├── systemd-service/
│   └── my-daemon/               # systemd service package
│       ├── src/main.c           # Daemon source code
│       └── debian/              # Packaging: control, rules, compat,
│                                #   changelog, my-daemon.service,
│                                #   postinst, prerm, source/format
├── ebpf-program/
│   └── my-bpf-sensor/           # CO-RE eBPF package
│       ├── src/                 # bpf_program.bpf.c + loader.c
│       ├── Makefile             # BPF build pipeline (clang → bpftool → gcc)
│       └── debian/              # Packaging: control, rules, compat,
│                                #   changelog, my-bpf-sensor.service,
│                                #   postinst, prerm, source/format
└── kernel-module/
    └── my-hello-module/         # Kernel module package
        ├── src/hello.c          # Module source code
        ├── Makefile             # Kbuild-compatible Makefile
        ├── dkms.conf            # DKMS configuration
        └── debian/              # Packaging: control, rules, compat,
                                 #   changelog, postinst, prerm, source/format
```

## Build & Install Verification

### Systemd Service (my-daemon)

```bash
cd systemd-service/my-daemon
dpkg-buildpackage -us -uc -b
sudo apt install ../my-daemon_*.deb
sudo apt remove my-daemon
```

Files installed: `/usr/sbin/my-daemon`, `/usr/lib/systemd/system/my-daemon.service`

**Generated artifacts:**
- `my-daemon_1.0.0-1_amd64.deb` — Binary package
- `my-daemon_1.0.0-1.dsc` — Source package descriptor
- `my-daemon_1.0.0.orig.tar.gz` — Original source archive
- `my-daemon_1.0.0-1.debian.tar.xz` — Debian patches and config
- `my-daemon_1.0.0-1_amd64.buildinfo` — Build information
- `my-daemon_1.0.0-1_amd64.changes` — Package changes file

### CO-RE eBPF Sensor (my-bpf-sensor)

```bash
cd ebpf-program/my-bpf-sensor
dpkg-buildpackage -us -uc -b
sudo apt install ../my-bpf-sensor_*.deb
sudo apt remove my-bpf-sensor
```

Files installed: `/usr/sbin/my-bpf-sensor`, `/usr/lib/systemd/system/my-bpf-sensor.service`  
Requires: `clang`, `llvm`, `libbpf-dev`, `bpftool`, kernel with BTF (`CONFIG_DEBUG_INFO_BTF=y`)

**Generated artifacts:**
- `my-bpf-sensor_1.0.0-1_amd64.deb` — Binary package
- `my-bpf-sensor_1.0.0-1.dsc` — Source package descriptor
- `my-bpf-sensor_1.0.0.orig.tar.gz` — Original source archive
- `my-bpf-sensor_1.0.0-1.debian.tar.xz` — Debian patches and config
- `my-bpf-sensor_1.0.0-1_amd64.buildinfo` — Build information
- `my-bpf-sensor_1.0.0-1_amd64.changes` — Package changes file

### Kernel Module (my-hello-module)

```bash
cd kernel-module/my-hello-module
dpkg-buildpackage -us -uc -b
sudo apt install ../my-hello-module_*.deb
sudo apt remove my-hello-module
```

Source installed: `/usr/src/my-hello-module-1.0.0/` (registered with DKMS)  
Module built by DKMS for: current + new kernels automatically

**Generated artifacts:**
- `my-hello-module_1.0.0-1_amd64.deb` — Binary package
- `my-hello-module_1.0.0-1.dsc` — Source package descriptor
- `my-hello-module_1.0.0.orig.tar.gz` — Original source archive
- `my-hello-module_1.0.0-1.debian.tar.xz` — Debian patches and config
- `my-hello-module_1.0.0-1_amd64.buildinfo` — Build information
- `my-hello-module_1.0.0-1_amd64.changes` — Package changes file
