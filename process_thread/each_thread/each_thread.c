#include<linux/init.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<linux/list.h>

#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

static int __init each_thread_init(void)
{
    struct task_struct *p = NULL, *t = NULL;
    int total = 0, kthread=0; 
pr_info("Insertion Successful.\n");

pr_info(" NAME   |  PID   | TGID    | STATE  \n");

for_each_process_thread(p,t) {
printk(KERN_INFO "%s | %6d  | %6d  | %c   \n",p->comm,task_pid_nr(p),task_tgid_nr(p),task_state_to_char(p));

if( t->mm == NULL) //kernel thread has no mapping in the user space.
kthread++;

total++;
}

pr_info("Total  number of thread :%6d \n kernel thread: %6d",total,kthread);

return 0;
}


static void __exit each_thread_exit(void)
{
pr_info("Removal Successful.\n");
}

module_init(each_thread_init);
module_exit(each_thread_exit);

MODULE_LICENSE("GPL 2");
MODULE_DESCRIPTION("Print each threads details.");
MODULE_AUTHOR("Dipendra Khakda");

