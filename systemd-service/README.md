# systemd-service — Packaging a systemd Daemon with APT

This directory contains a sample Debian package (`my-daemon`) that demonstrates how to bundle a user-space daemon with a systemd service unit for installation via APT.

## Package: `my-daemon`

A simple daemon written in C that logs a "service is running" message every 30 seconds via syslog. It is managed by systemd and demonstrates best practices for APT-packaged services.

### What This Demonstrates

| Concept | Implementation |
|---------|---------------|
| **Binary installation** | `debian/rules` compiles `src/main.c` and installs to `/usr/sbin/my-daemon` |
| **systemd service unit** | `debian/my-daemon.service` is installed to `/lib/systemd/system/` |
| **Auto-enable on install** | `debian/postinst` calls `deb-systemd-helper enable/start` |
| **Auto-stop on remove** | `debian/prerm` calls `deb-systemd-helper stop` |
| **Service hardening** | `my-daemon.service` uses `NoNewPrivileges`, `PrivateTmp`, `ProtectSystem` |
| **Graceful shutdown** | `src/main.c` handles `SIGTERM`/`SIGINT` for clean syslog closure |

### Debian Packaging Files

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata with `systemd` dependency |
| `debian/rules` | Build recipe using `dh` + custom override targets |
| `debian/compat` | Debhelper compatibility level 13 |
| `debian/source/format` | Source format `3.0 (quilt)` |
| `debian/install` | Installs the `.service` file to `lib/systemd/system/` |
| `debian/my-daemon.service` | systemd unit file for the daemon |
| `debian/postinst` | Enables and starts the service after installation |
| `debian/prerm` | Stops the service before removal |

### Build & Install

```bash
cd systemd-service/my-daemon

# Install build dependencies
sudo apt build-dep ./

# Build the binary package
dpkg-buildpackage -us -uc -b

# Install
sudo apt install ../my-daemon_*.deb

# Verify the service is running
systemctl status my-daemon
journalctl -u my-daemon -f

# Remove
sudo apt remove my-daemon
```

### How It Works

1. **Install**: `apt install my-daemon` copies the binary to `/usr/sbin/` and the `.service` to `/lib/systemd/system/`, then runs `postinst` which calls `systemctl enable --now my-daemon`.
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