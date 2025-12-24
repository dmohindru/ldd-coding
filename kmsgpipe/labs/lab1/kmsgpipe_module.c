#include <linux/module.h>
#include "kmsgpipe_module.h"

/* local wrappers (required!) */
static int __init kmsgpipe_init(void)
{
    return kmsgpipe_module_init();
}

static void __exit kmsgpipe_exit(void)
{
    kmsgpipe_module_exit();
}

module_init(kmsgpipe_init);
module_exit(kmsgpipe_exit);

MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("kmsgpipe global character driver");
