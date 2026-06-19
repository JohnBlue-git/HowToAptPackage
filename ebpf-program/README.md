# ebpf-program — Packaging a CO-RE eBPF Program with APT

This directory contains a sample Debian package (`my-bpf-sensor`) that demonstrates how to bundle a CO-RE (Compile Once - Run Everywhere) eBPF program for installation via APT.

## Package: `my-bpf-sensor`

An eBPF-based process exec monitor that traces `execve` syscalls and reports them to user-space via a ring buffer. It uses the CO-RE approach with BTF (BPF Type Format) to work across different kernel versions without modification.

### What This Demonstrates

| Concept | Implementation |
|---------|---------------|
| **CO-RE eBPF** | BPF program compiled with `-target bpf` and BTF debug info |
| **BPF skeleton** | `bpftool gen skeleton` generates a header for the user-space loader |
| **libbpf integration** | User-space loader uses `libbpf` APIs: open, load, attach, ring_buffer |
| **Systemd-managed eBPF** | The loader runs as a systemd service with `CAP_BPF` capability |
| **Kernel BTF dependency** | Package depends on `libbpf1` and recommends a BTF-enabled kernel |
| **Build-time tools** | Requires `clang`, `llvm`, `bpftool`, `libbpf-dev` to build |

### Debian Packaging Files

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata with `libbpf1`, `clang`, `bpftool` build-deps |
| `debian/rules` | Build recipe calling the project `Makefile` |
| `debian/source/format` | Source format `3.0 (quilt)` |
| `debian/install` | Installs the `.service` file to `lib/systemd/system/` |
| `debian/my-bpf-sensor.service` | systemd unit with `CAP_BPF` capabilities |
| `debian/postinst` | Enables and starts the sensor service |
| `debian/prerm` | Stops the sensor service before removal |

### Build & Install

```bash
cd ebpf-program/my-bpf-sensor

# Install build dependencies
sudo apt install clang llvm libbpf-dev bpftool linux-headers-generic
sudo apt build-dep ./

# Build the binary package
dpkg-buildpackage -us -uc -b

# Install
sudo apt install ../my-bpf-sensor_*.deb

# Verify the sensor is running
systemctl status my-bpf-sensor
journalctl -u my-bpf-sensor -f

# Test: run a command and watch the eBPF events
ls /tmp
journalctl -u my-bpf-sensor --since "1 minute ago"

# Remove
sudo apt remove my-bpf-sensor
```

### Build Pipeline

```
bpf_program.bpf.c          # BPF C source (runs in kernel)
       │
       ▼ clang -target bpf -g -O2
bpf_program.bpf.o          # BPF object file with embedded BTF
       │
       ▼ bpftool gen skeleton
bpf_program.skel.h         # Generated C header: BPF skeleton
       │
       ▼ gcc (with libbpf)
loader.c ──────────────────► my-bpf-sensor (user-space binary)
```

### How CO-RE Works

1. **Compile with BTF**: The BPF program is compiled with `-g` to embed BTF (BPF Type Format) information. BTF describes kernel data structures in a version-agnostic way.

2. **Generate skeleton**: `bpftool gen skeleton` creates a C header file that provides functions like `my_bpf_sensor_bpf__open()`, `__load()`, `__attach()`. This skeleton handles CO-RE relocations automatically.

3. **Runtime relocation**: When `libbpf` loads the BPF object, it reads BTF from both the object file and the running kernel (`/sys/kernel/btf/vmlinux`). It then relocates field offsets — for example, if `task_struct->pid` moved between kernel versions, libbpf adjusts the offset automatically.

4. **Result**: The same `.deb` works on kernel 5.4, 5.10, 5.15, 6.x, etc. — no recompilation needed.

### Required Kernel Features

- `CONFIG_DEBUG_INFO_BTF=y` — Provides kernel BTF for CO-RE relocations
- `CONFIG_BPF=y` — BPF subsystem
- `CONFIG_BPF_SYSCALL=y` — BPF syscall interface
- Check with: `cat /proc/config.gz | gunzip | grep CONFIG_DEBUG_INFO_BTF`

### Service Unit Highlights

```ini
[Service]
ExecStart=/usr/sbin/my-bpf-sensor
User=root

# eBPF requires specific Linux capabilities:
CapabilityBoundingSet=CAP_BPF CAP_SYS_ADMIN CAP_NET_ADMIN
AmbientCapabilities=CAP_BPF CAP_SYS_ADMIN CAP_NET_ADMIN

# For kernels < 5.8 (before CAP_BPF was added), CAP_SYS_ADMIN is needed.
# CAP_NET_ADMIN is required for network-related BPF programs.