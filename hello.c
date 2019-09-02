/**
 * @file    hello.c
 * @author  Cristian Aldea, Dan Seremet
 * @date    2 September 2019
 * @version 1.0
 * @brief  A simple LKM
 * @see 
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cristian Aldea, Dan Seremet");
MODULE_DESCRIPTION("A simple kernel module");
MODULE_VERSION("1.0");

static char *name = "world";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  // parameter description

static int __init hello_init(void){
   printk(KERN_INFO "EBB: Hello %s from the BBB LKM!\n", name);
   return 0;
}

static void __exit hello_exit(void){
   printk(KERN_INFO "EBB: Goodbye %s from the BBB LKM!\n", name);
}

/** @brief Using the "module_init()" and "module_exit()" macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function.
 */
module_init(hello_init);
module_exit(hello_exit);