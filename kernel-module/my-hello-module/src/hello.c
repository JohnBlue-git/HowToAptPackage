/*
 * my-hello-module - A sample Linux kernel module packaged via APT
 *
 * This is a simple kernel module that creates /proc/myhello and logs
 * a message when loaded/unloaded. It demonstrates kernel module
 * packaging with DKMS support for Debian packages.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define MODULE_NAME "myhello"
#define PROCFS_NAME "myhello"

static struct proc_dir_entry *proc_entry;
static char module_msg[] = "Hello from my-hello-module (APT-packaged kernel module)\n";

/*
 * Read handler for /proc/myhello
 * Returns the module greeting message.
 */
static ssize_t proc_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(buf, count, ppos,
                                   module_msg, strlen(module_msg));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};
#else
static const struct file_operations proc_fops = {
    .read = proc_read,
};
#endif

/*
 * Module initialization
 * Creates /proc/myhello entry.
 */
static int __init hello_init(void)
{
    proc_entry = proc_create(PROCFS_NAME, 0444, NULL, &proc_fops);
    if (!proc_entry) {
        pr_err("%s: Failed to create /proc/%s\n", MODULE_NAME, PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("%s: Module loaded. Run: cat /proc/%s\n", MODULE_NAME, PROCFS_NAME);
    return 0;
}

/*
 * Module cleanup
 * Removes /proc/myhello entry.
 */
static void __exit hello_exit(void)
{
    remove_proc_entry(PROCFS_NAME, NULL);
    pr_info("%s: Module unloaded.\n", MODULE_NAME);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HowToAptPackage Maintainer <maintainer@example.com>");
MODULE_DESCRIPTION("Sample kernel module packaged via APT with DKMS support");
MODULE_VERSION("1.0.0");