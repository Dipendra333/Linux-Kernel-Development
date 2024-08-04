#include<linux/init.h>
#include<linux/module.h>

#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
// Module Parameters

static char *name;
module_param(name,charp,0660);

static int age;
module_param(age,int,0660);

static int __init hello_world_init(void)
{

pr_info("Insertion Successful!\n");
pr_info("%s is %d years old\n",name,age);
return 0;
}

static void __exit hello_world_exit(void)
{
pr_info("Removal Successful!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

// Module Information
MODULE_LICENSE("GPL 2");
MODULE_DESCRIPTION("Module Parameter");
MODULE_AUTHOR("Dipendra Khakda");