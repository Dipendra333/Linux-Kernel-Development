#include<linux/init.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<linux/list.h>

#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

static int __init each_process_init(void)
{
    struct task_struct *p;
    int total = 0;
pr_info("Insertion Successful.\n");

pr_info(" NAME   |  PID   | TGID    | STATE  \n");

for_each_process(p) {
printk(KERN_INFO "%s | %6d  | %6d  | %c   \n",p->comm,task_pid_nr(p),task_tgid_nr(p),task_state_to_char(p));
total++;
}

pr_info("Total  number of processes :%5d \n",total);

return 0;
}
/*
current->state may be;
R:running
S: sleeping. 1. interruptible and uninterruptible
I: idle

Z: zombie
T: terminiated(stopped)
t: trace
X: Dead
D: Disk sleep
W: Waiting
*/

static void __exit each_process_exit(void)
{
pr_info("Removal Successful.\n");
}

module_init(each_process_init);
module_exit(each_process_exit);

MODULE_LICENSE("GPL 2");
MODULE_DESCRIPTION("Print each process details.");
MODULE_AUTHOR("Dipendra Khakda");

