#include<linux/init.h>
#include<linux/module.h>

static int __init hello_world_init(void)
{
pr_info("Hello world!\n");
return 0;
}

static void __exit hello_world_exit(void)
{
pr_info("By world\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL 2");
MODULE_DESCRIPTION("Hello World Module");
MODULE_AUTHOR("Dipendra Khakda");
