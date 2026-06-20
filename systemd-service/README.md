# systemd-service — Packaging a systemd Daemon with APT

This directory contains a sample Debian package (`my-daemon`) that demonstrates how to bundle a user-space daemon with a systemd service unit for installation via APT.

## Package: `my-daemon`

A simple daemon written in C that logs a "service is running" message every 30 seconds via syslog. It is managed by systemd and demonstrates best practices for APT-packaged services.

### Verified Build & Install

```bash
cd systemd-service/my-daemon

# Install build dependencies
sudo apt install debhelper gcc fakeroot
sudo apt build-dep ./

# Build the binary package
dpkg-buildpackage -us -uc -b

# Install
sudo apt install ../my-daemon_*.deb

# Verify the files are installed
ls -la /usr/sbin/my-daemon
ls -la /usr/lib/systemd/system/my-daemon.service

# Remove
sudo apt remove my-daemon
```

### Generated Artifacts

When you run `dpkg-buildpackage`, the following files are generated in the parent directory:

**Binary Package:**
- `my-daemon_1.0.0-1_amd64.deb` — The installable Debian package containing the compiled daemon binary and systemd service unit

**Build Metadata:**
- `my-daemon_1.0.0-1.dsc` — Source package descriptor (lists source files, checksums, dependencies)
- `my-daemon_1.0.0.orig.tar.gz` — Original source archive
- `my-daemon_1.0.0-1.debian.tar.xz` — Debian-specific patches and configuration files (from `debian/` directory)
- `my-daemon_1.0.0-1_amd64.buildinfo` — Build machine and environment information
- `my-daemon_1.0.0-1_amd64.changes` — Summary of changes and package contents

**Inside the `.deb` Package (visible after `dpkg -X`):**
- `usr/sbin/my-daemon` — Compiled daemon binary
- `usr/lib/systemd/system/my-daemon.service` — systemd service unit file
- `usr/share/doc/my-daemon/` — Documentation files (changelog, copyright)
- `DEBIAN/control` — Package metadata
- `DEBIAN/postinst` — Post-installation script (enables and starts the service)
- `DEBIAN/prerm` — Pre-removal script (stops the service)

### What This Demonstrates

| Concept | Implementation |
|---------|---------------|
| **Binary installation** | `debian/rules` compiles `src/main.c` and installs to `/usr/sbin/my-daemon` |
| **systemd service unit** | `debian/my-daemon.service` is installed by `dh_installsystemd` |
| **Auto-enable on install** | `debian/postinst` calls `deb-systemd-helper enable/start` |
| **Auto-stop on remove** | `debian/prerm` calls `deb-systemd-helper stop` |
| **Service hardening** | `my-daemon.service` uses `NoNewPrivileges`, `PrivateTmp`, `ProtectSystem` |
| **Graceful shutdown** | `src/main.c` handles `SIGTERM`/`SIGINT` for clean syslog closure |

### Debian Packaging Files

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata with `systemd` dependency |
| `debian/rules` | Build recipe using `dh` + custom override targets |
| `debian/changelog` | **Required** — version history (was missing, added) |
| `debian/compat` | Debhelper compatibility level 13 |
| `debian/source/format` | Source format `3.0 (quilt)` |
| `debian/my-daemon.service` | systemd unit file for the daemon |
| `debian/postinst` | Enables and starts the service after installation |
| `debian/prerm` | Stops the service before removal |

> **Note:** We do NOT use `debian/install` — `dh_installsystemd` automatically detects and installs the `.service` file. An explicit `debian/install` would cause duplicate file conflicts on merged-usr systems where `/lib` is a symlink to `/usr/lib`.

### How It Works

1. **Install**: `apt install my-daemon` copies the binary to `/usr/sbin/` and the `.service` to `/usr/lib/systemd/system/`, then runs `postinst` which calls `systemctl enable --now my-daemon`.
2. **Runtime**: The daemon logs to syslog every 30 seconds. It responds to `SIGTERM` for graceful shutdown.
3. **Remove**: `apt remove my-daemon` runs `prerm` which stops the service via `systemctl stop`, then removes all files.

### Service Unit Breakdown

```ini
[Unit]
Description=My Daemon - Sample systemd service from APT package
After=network.target           # Start after networking is up

[Service]
Type=simple                    # Main process is the daemon itself
ExecStart=/usr/sbin/my-daemon  # Path to the binary
Restart=on-failure             # Auto-restart if it crashes
RestartSec=10                  # Wait 10s before restart
User=daemon                    # Run as unprivileged 'daemon' user
Group=daemon

# Security hardening
NoNewPrivileges=yes            # Prevent privilege escalation
PrivateTmp=yes                 # Isolated /tmp
ProtectSystem=full             # Read-only /usr and /etc
ProtectHome=yes                # Hide /home

[Install]
WantedBy=multi-user.target     # Start in normal multi-user mode
```
