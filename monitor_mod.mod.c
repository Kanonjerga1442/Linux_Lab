#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x055d18ae, "proc_remove" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xc3781560, "kmalloc_caches" },
	{ 0x5bff20d5, "__kmalloc_cache_noprof" },
	{ 0x5cb46e6d, "validate_usercopy_range" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xbeb1d261, "__flush_workqueue" },
	{ 0xbeb1d261, "destroy_workqueue" },
	{ 0xd272d446, "__fentry__" },
	{ 0x1c77f2cc, "proc_create" },
	{ 0xe8213e80, "_printk" },
	{ 0xdf4bee3d, "alloc_workqueue_noprof" },
	{ 0xa3ed642b, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x055d18ae,
	0xd272d446,
	0xcb8b6ec6,
	0xbd03ed67,
	0xc3781560,
	0x5bff20d5,
	0x5cb46e6d,
	0xa61fd7aa,
	0x092a35a2,
	0x49733ad6,
	0x90a48d82,
	0xbeb1d261,
	0xbeb1d261,
	0xd272d446,
	0x1c77f2cc,
	0xe8213e80,
	0xdf4bee3d,
	0xa3ed642b,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"proc_remove\0"
	"__x86_return_thunk\0"
	"kfree\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"validate_usercopy_range\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"queue_work_on\0"
	"__ubsan_handle_out_of_bounds\0"
	"__flush_workqueue\0"
	"destroy_workqueue\0"
	"__fentry__\0"
	"proc_create\0"
	"_printk\0"
	"alloc_workqueue_noprof\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0E96F54DB1191E8657DDF29");
