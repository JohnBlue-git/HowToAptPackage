/*
 * my-bpf-sensor - A CO-RE (Compile Once - Run Everywhere) eBPF program
 *
 * This eBPF program traces process exec events and is packaged via APT.
 * It uses BTF (BPF Type Format) for CO-RE compatibility across kernel versions.
 */

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char LICENSE[] SEC("license") = "GPL";

/* Define a map to store exec events */
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024); /* 256 KB */
} exec_events SEC(".maps");

/* Event structure passed to user-space */
struct exec_event {
    __u32 pid;
    __u32 uid;
    char comm[16];
};

/*
 * Tracepoint: syscalls/sys_enter_execve
 *
 * Uses CO-RE via BPF skeleton and BTF information.
 * The bpf_raw_tracepoint_open() loader handles CO-RE relocations.
 */
SEC("tracepoint/syscalls/sys_enter_execve")
int trace_execve(struct trace_event_raw_sys_enter *ctx)
{
    struct exec_event *event;
    __u32 pid;
    __u32 uid;

    pid = bpf_get_current_pid_tgid() >> 32;
    uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;

    /* Reserve space in the ring buffer */
    event = bpf_ringbuf_reserve(&exec_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->pid = pid;
    event->uid = uid;

    /* Read the comm (process name) using CO-RE helpers */
    bpf_get_current_comm(&event->comm, sizeof(event->comm));

    bpf_ringbuf_submit(event, 0);
    return 0;
}
