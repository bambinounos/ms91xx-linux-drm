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
	{ 0xbee76e2a, "usb_alloc_urb" },
	{ 0xd6e79690, "down_timeout" },
	{ 0x887ecaba, "usb_free_urb" },
	{ 0x57860fb4, "wait_for_completion_timeout" },
	{ 0x12f6a6e7, "devm_kmalloc" },
	{ 0x6528d32c, "vmalloc_to_page" },
	{ 0xcba5f8e2, "usb_alloc_coherent" },
	{ 0x40a621c5, "snprintf" },
	{ 0x65026e43, "complete" },
	{ 0x562e3aaa, "__kfifo_in" },
	{ 0xd648ae19, "sg_free_table" },
	{ 0x91d29553, "msdisp_drm_get_free_device" },
	{ 0x60c9c0b3, "__init_swait_queue_head" },
	{ 0x756dc180, "usb_register_driver" },
	{ 0x4073d0de, "up" },
	{ 0xa53f4e29, "memcpy" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x184529f1, "get_device" },
	{ 0xd272d446, "__fentry__" },
	{ 0x052ead79, "msdisp_drm_get_pipeline_global_id" },
	{ 0xdba15b90, "wake_up_process" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0xf00d45ac, "kstrtou16" },
	{ 0xe8213e80, "_printk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x9ffccd00, "usb_bulk_msg" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xd4a4112c, "put_device" },
	{ 0xd710adbf, "__kmalloc_large_noprof" },
	{ 0x07f964c6, "msdisp_drm_get_kfifo" },
	{ 0x9479a1e8, "strnlen" },
	{ 0xb3cfa727, "usb_submit_urb" },
	{ 0xcac6bbbd, "_dev_info" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xe118f9e0, "sysfs_create_link" },
	{ 0xcac6bbbd, "_dev_err" },
	{ 0x51c7a676, "usb_free_coherent" },
	{ 0x76436691, "drm_mode_config_helper_suspend" },
	{ 0x471ae204, "sysfs_remove_link" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xd7a59a65, "vmalloc_noprof" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x95bff530, "usb_control_msg" },
	{ 0xc6badcf4, "sysfs_create_group" },
	{ 0x173ec8da, "sscanf" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x91f40251, "usb_deregister" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0x6aac932b, "msdisp_drm_get_free_pipeline_index" },
	{ 0x8ebe8086, "sg_alloc_table_from_pages_segment" },
	{ 0x27683a56, "memset" },
	{ 0xcac6bbbd, "_dev_warn" },
	{ 0x5a844b26, "__x86_indirect_thunk_r10" },
	{ 0x4d8419c6, "param_ops_charp" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x357aaab3, "mutex_trylock" },
	{ 0x3855aac9, "kthread_create_on_node" },
	{ 0xce4af33b, "kstrdup" },
	{ 0x3cf61928, "sysfs_remove_group" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x82fd7238, "__ubsan_handle_shift_out_of_bounds" },
	{ 0xf1de9e85, "vfree" },
	{ 0xa5c7582d, "strsep" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xecd17989, "__kmalloc_cache_noprof" },
	{ 0x887ecaba, "usb_kill_urb" },
	{ 0x97acb853, "ktime_get" },
	{ 0x5a844b26, "__x86_indirect_thunk_r9" },
	{ 0x2ca3e397, "msdisp_drm_register_usb_hal" },
	{ 0xce16474d, "msdisp_drm_get_pipeline_kobject" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0x43a349ca, "strlen" },
	{ 0x76436691, "drm_mode_config_helper_resume" },
	{ 0xd1ea1c88, "__kfifo_out" },
	{ 0x5a844b26, "__x86_indirect_thunk_r8" },
	{ 0x67628f51, "msleep" },
	{ 0x08bfc903, "kmalloc_caches" },
	{ 0x052ead79, "msdisp_drm_unregister_usb_hal" },
	{ 0x814e12e5, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xbee76e2a,
	0xd6e79690,
	0x887ecaba,
	0x57860fb4,
	0x12f6a6e7,
	0x6528d32c,
	0xcba5f8e2,
	0x40a621c5,
	0x65026e43,
	0x562e3aaa,
	0xd648ae19,
	0x91d29553,
	0x60c9c0b3,
	0x756dc180,
	0x4073d0de,
	0xa53f4e29,
	0xcb8b6ec6,
	0x184529f1,
	0xd272d446,
	0x052ead79,
	0xdba15b90,
	0x5a844b26,
	0xf00d45ac,
	0xe8213e80,
	0xbd03ed67,
	0x9ffccd00,
	0xd272d446,
	0xd4a4112c,
	0xd710adbf,
	0x07f964c6,
	0x9479a1e8,
	0xb3cfa727,
	0xcac6bbbd,
	0x90a48d82,
	0xe118f9e0,
	0xcac6bbbd,
	0x51c7a676,
	0x76436691,
	0x471ae204,
	0xbd03ed67,
	0xd7a59a65,
	0xf46d5bf3,
	0x95bff530,
	0xc6badcf4,
	0x173ec8da,
	0xc1e6c71e,
	0x91f40251,
	0xe54e0a6b,
	0x6aac932b,
	0x8ebe8086,
	0x27683a56,
	0xcac6bbbd,
	0x5a844b26,
	0x4d8419c6,
	0xd272d446,
	0x357aaab3,
	0x3855aac9,
	0xce4af33b,
	0x3cf61928,
	0xdd6830c7,
	0x82fd7238,
	0xf1de9e85,
	0xa5c7582d,
	0xf46d5bf3,
	0xecd17989,
	0x887ecaba,
	0x97acb853,
	0x5a844b26,
	0x2ca3e397,
	0xce16474d,
	0xe4de56b4,
	0x43a349ca,
	0x76436691,
	0xd1ea1c88,
	0x5a844b26,
	0x67628f51,
	0x08bfc903,
	0x052ead79,
	0x814e12e5,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"usb_alloc_urb\0"
	"down_timeout\0"
	"usb_free_urb\0"
	"wait_for_completion_timeout\0"
	"devm_kmalloc\0"
	"vmalloc_to_page\0"
	"usb_alloc_coherent\0"
	"snprintf\0"
	"complete\0"
	"__kfifo_in\0"
	"sg_free_table\0"
	"msdisp_drm_get_free_device\0"
	"__init_swait_queue_head\0"
	"usb_register_driver\0"
	"up\0"
	"memcpy\0"
	"kfree\0"
	"get_device\0"
	"__fentry__\0"
	"msdisp_drm_get_pipeline_global_id\0"
	"wake_up_process\0"
	"__x86_indirect_thunk_rax\0"
	"kstrtou16\0"
	"_printk\0"
	"__ref_stack_chk_guard\0"
	"usb_bulk_msg\0"
	"__stack_chk_fail\0"
	"put_device\0"
	"__kmalloc_large_noprof\0"
	"msdisp_drm_get_kfifo\0"
	"strnlen\0"
	"usb_submit_urb\0"
	"_dev_info\0"
	"__ubsan_handle_out_of_bounds\0"
	"sysfs_create_link\0"
	"_dev_err\0"
	"usb_free_coherent\0"
	"drm_mode_config_helper_suspend\0"
	"sysfs_remove_link\0"
	"random_kmalloc_seed\0"
	"vmalloc_noprof\0"
	"mutex_lock\0"
	"usb_control_msg\0"
	"sysfs_create_group\0"
	"sscanf\0"
	"__mutex_init\0"
	"usb_deregister\0"
	"__fortify_panic\0"
	"msdisp_drm_get_free_pipeline_index\0"
	"sg_alloc_table_from_pages_segment\0"
	"memset\0"
	"_dev_warn\0"
	"__x86_indirect_thunk_r10\0"
	"param_ops_charp\0"
	"__x86_return_thunk\0"
	"mutex_trylock\0"
	"kthread_create_on_node\0"
	"kstrdup\0"
	"sysfs_remove_group\0"
	"sprintf\0"
	"__ubsan_handle_shift_out_of_bounds\0"
	"vfree\0"
	"strsep\0"
	"mutex_unlock\0"
	"__kmalloc_cache_noprof\0"
	"usb_kill_urb\0"
	"ktime_get\0"
	"__x86_indirect_thunk_r9\0"
	"msdisp_drm_register_usb_hal\0"
	"msdisp_drm_get_pipeline_kobject\0"
	"__ubsan_handle_load_invalid_value\0"
	"strlen\0"
	"drm_mode_config_helper_resume\0"
	"__kfifo_out\0"
	"__x86_indirect_thunk_r8\0"
	"msleep\0"
	"kmalloc_caches\0"
	"msdisp_drm_unregister_usb_hal\0"
	"module_layout\0"
;

MODULE_INFO(depends, "usbdisp_drm");

MODULE_ALIAS("usb:v345Fp9132d*dc*dsc*dp*icFFisc00ip00in*");
MODULE_ALIAS("usb:v345Fp9133d*dc*dsc*dp*icFFisc00ip00in*");
MODULE_ALIAS("usb:v345Fp9135d*dc*dsc*dp*icFFisc00ip00in*");

MODULE_INFO(srcversion, "5CBE9D879BE6A26589C3844");
