#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/param.h>
#include <linux/platform_device.h>
#include <linux/err.h>

#define MAX_DURATION_SEC 300

static struct timer_list g_timer;
static struct platform_device *pdev;
static unsigned long start_time;
static unsigned int interval_sec = 30;

static void timer_callback(struct timer_list *t)
{
    unsigned long elapsed = jiffies - start_time;
    unsigned int minutes = elapsed / secs_to_jiffies(60);

     if (elapsed >= secs_to_jiffies(MAX_DURATION_SEC))
         return;

    pr_info("min=%d: Hello, timer!\n", minutes);

    mod_timer(&g_timer, jiffies + secs_to_jiffies(interval_sec));
}

static ssize_t interval_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%u\n", READ_ONCE(interval_sec));
}

static ssize_t interval_store(struct device *dev, struct device_attribute *attr,
                              const char *buf, size_t count)
{
    unsigned int new_interval;
    if (kstrtouint(buf, 10, &new_interval) || new_interval == 0)
        return -EINVAL;

    interval_sec = new_interval;

    WRITE_ONCE(interval_sec, new_interval);
    mod_timer(&g_timer, jiffies + secs_to_jiffies(interval_sec));

    pr_info("interval changed to %u sec\n", new_interval);
    return count;
}

static DEVICE_ATTR_RW(interval);

static int __init kernel_timer_init(void) {
    int ret;

    pdev = platform_device_register_simple("kernel_timer", -1, NULL, 0);
    if (IS_ERR(pdev)) {
        ret = PTR_ERR(pdev);
        pr_err("failed to register platform device: %d\n", ret);
        return ret;
    }

    ret = device_create_file(&pdev->dev, &dev_attr_interval);
    if (ret) {
        pr_err("failed to create sysfs attribute: %d\n", ret);
        platform_device_unregister(pdev);
        return ret;
    }

    start_time = jiffies;
    timer_setup(&g_timer, timer_callback, 0);
    mod_timer(&g_timer, jiffies + secs_to_jiffies(interval_sec));

    pr_info("kernel_timer loaded successfully: interval_sec=%u, max_duration=%u", interval_sec, MAX_DURATION_SEC);
    return 0;
}

static void __exit kernel_timer_exit(void) {
    del_timer_sync(&g_timer);
    device_remove_file(&pdev->dev, &dev_attr_interval);
    platform_device_unregister(pdev);
    pr_info("kernel_timer unloaded\n");
}

module_init(kernel_timer_init);
module_exit(kernel_timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("timer_list Kernel Module");
MODULE_VERSION("1.0");
