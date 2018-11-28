#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xef894695, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb02fd006, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0x25e5fb21, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x5f25fde6, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xd8e484f0, __VMLINUX_SYMBOL_STR(register_chrdev_region) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x272a93ec, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x17661d23, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x9166146e, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x8329e6f0, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0x61651be, __VMLINUX_SYMBOL_STR(strcat) },
	{ 0x78e739aa, __VMLINUX_SYMBOL_STR(up) },
	{ 0x6dc0c9dc, __VMLINUX_SYMBOL_STR(down_interruptible) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x139b504, __VMLINUX_SYMBOL_STR(cpu_current_top_of_stack) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "698BA923A7F6F85981E4835");
