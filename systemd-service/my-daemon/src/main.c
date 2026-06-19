/*
 * my-daemon - A sample systemd service managed via APT
 *
 * This is a simple daemon that logs a message periodically.
 * It demonstrates how a systemd service is bundled in a Debian package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>

static volatile int keep_running = 1;

void handle_signal(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        keep_running = 0;
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    int iteration = 0;

    /* Open syslog for logging */
    openlog("my-daemon", LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_INFO, "my-daemon starting (version 1.0.0)");

    /* Set up signal handlers for graceful shutdown */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    /* Main loop - log a message every 30 seconds */
    while (keep_running) {
        syslog(LOG_INFO, "my-daemon iteration %d: service is running", iteration++);
        sleep(30);
    }

    syslog(LOG_INFO, "my-daemon shutting down gracefully");
    closelog();
    return EXIT_SUCCESS;
}