#include<linux/init.h>
#include<linux/module.h>

// Module Parameters

static char *name;
module_param(name,charp,0660);
MODULE_PARAM_DESC(name,"Name of the user.");

static int age;
module_param(age,int,0660);
MODULE_PARAM_DESC(age,"Age of the user");

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