#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int int_val;

static int int_val_set(const char *val, const struct kernel_param *kp)
{
    int ret;
    ret = kstrtoint(val, 10, &int_val);
    if(ret)
    {
        pr_err("kstrtoint error\n");
        return ret;
    }

    pr_info("int_val value = %d\n", int_val);
    return 0;
}

static int int_val_get(char *val, const struct kernel_param *kp)
{
    return sprintf(val, "%d\n", int_val);
}

static const struct kernel_param_ops int_val_params =
{
    .set = int_val_set,
    .get = int_val_get,
};


module_param_cb(int_val, &int_val_params, &int_val, 0664);
MODULE_PARM_DESC(int_val, "Value to change from sysfs");

static int __init hello_init(void)
{
    pr_info("Hello World from kernel!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("Goodbye from kernel!\n");
}

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Viacheslav Stepanov");
MODULE_DESCRIPTION("Simple print module");
MODULE_VERSION("1.2");