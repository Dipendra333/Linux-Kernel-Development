#include<linux/init.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<linux/delay.h>

#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

static void show_current(void)
{
 pr_info("Process Detail:\n"
    " name : %s  \n"
    " PID  : %6d \n"
    " TGID : %6d \n"  
    " State: %c  \n",
    current->comm, task_pid_nr(current), task_tgid_nr(current), task_state_to_char(current)
    // current->comm, current->pid, current->tgid,current->__state
 );
}

static int __init current_init(void)
{
pr_info("Insertion Successful.\n");
show_current();
msleep(1000);
return 0;
}

static void __exit current_exit(void)
{
 show_current();
 pr_info("Removal Successful.\n");
}

module_init(current_init);
module_exit(current_exit);

MODULE_LICENSE("GPL 2");
MODULE_DESCRIPTION("Print current thread");
MODULE_AUTHOR("Dipendra Khakda");

