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

KSYMTAB_FUNC(msdisp_platform_get_device, "", "");
KSYMTAB_FUNC(msdisp_platform_get_plat_device_index, "", "");
KSYMTAB_FUNC(msdisp_drm_get_free_device, "", "");
KSYMTAB_FUNC(msdisp_drm_get_drm_device_index, "", "");
KSYMTAB_FUNC(msdisp_drm_get_free_pipeline_index, "", "");
KSYMTAB_FUNC(msdisp_drm_get_pipeline_global_id, "", "");
KSYMTAB_FUNC(msdisp_drm_get_pipeline_kobject, "", "");
KSYMTAB_FUNC(msdisp_drm_register_usb_hal, "", "");
KSYMTAB_FUNC(msdisp_drm_unregister_usb_hal, "", "");
KSYMTAB_FUNC(msdisp_drm_get_kfifo, "", "");

SYMBOL_CRC(msdisp_platform_get_device, 0x194fd2f4, "");
SYMBOL_CRC(msdisp_platform_get_plat_device_index, 0x6b541a77, "");
SYMBOL_CRC(msdisp_drm_get_free_device, 0x91d29553, "");
SYMBOL_CRC(msdisp_drm_get_drm_device_index, 0x6aac932b, "");
SYMBOL_CRC(msdisp_drm_get_free_pipeline_index, 0x6aac932b, "");
SYMBOL_CRC(msdisp_drm_get_pipeline_global_id, 0x052ead79, "");
SYMBOL_CRC(msdisp_drm_get_pipeline_kobject, 0xce16474d, "");
SYMBOL_CRC(msdisp_drm_register_usb_hal, 0x2ca3e397, "");
SYMBOL_CRC(msdisp_drm_unregister_usb_hal, 0x052ead79, "");
SYMBOL_CRC(msdisp_drm_get_kfifo, 0x07f964c6, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x1bdf2bc8, "sme_me_mask" },
	{ 0xe4f8ce20, "drm_framebuffer_cleanup" },
	{ 0xf12c0013, "drm_atomic_helper_plane_destroy_state" },
	{ 0x22755710, "filp_open" },
	{ 0x0f0f139f, "drm_open" },
	{ 0x6b720f69, "drm_mode_vrefresh" },
	{ 0x8cede46e, "drm_poll" },
	{ 0x553959eb, "__drm_err" },
	{ 0x9f780415, "drm_gem_get_pages" },
	{ 0xd9a2bb0b, "dma_buf_vmap" },
	{ 0x62ee94dc, "drm_kms_helper_poll_init" },
	{ 0xfa552b22, "drm_framebuffer_init" },
	{ 0x9bb9323d, "drm_plane_enable_fb_damage_clips" },
	{ 0x143526e8, "dev_set_name" },
	{ 0xea97e6f1, "platform_driver_unregister" },
	{ 0x4d8419c6, "param_ops_ushort" },
	{ 0x5af83d53, "drm_atomic_add_affected_planes" },
	{ 0x212d603c, "dma_buf_end_cpu_access" },
	{ 0x033456f6, "drm_mode_probed_add" },
	{ 0xa10c73d3, "dma_resv_init" },
	{ 0xac823d72, "drm_atomic_helper_disable_plane" },
	{ 0xf6ed07f0, "__kernel_write" },
	{ 0x59c5f726, "drm_edid_free" },
	{ 0x30add0f2, "drm_prime_sg_to_page_array" },
	{ 0xd4a4112c, "device_initialize" },
	{ 0x5c12d4ca, "drm_mode_object_get" },
	{ 0xa53f4e29, "memcpy" },
	{ 0xf1de9e85, "vunmap" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x1e419e08, "drm_atomic_helper_crtc_destroy_state" },
	{ 0xc31b1c55, "drm_crtc_cleanup" },
	{ 0x417f4070, "drm_gem_create_mmap_offset" },
	{ 0x2352b148, "timer_delete" },
	{ 0x4c190cbe, "drm_encoder_cleanup" },
	{ 0xa96ff906, "drm_gem_object_init" },
	{ 0x283bce08, "__kfifo_free" },
	{ 0x0eaef536, "drmm_mode_config_init" },
	{ 0xe1e1f979, "_raw_spin_lock_irqsave" },
	{ 0xde338d9a, "_raw_spin_lock" },
	{ 0xd272d446, "__fentry__" },
	{ 0x5c12d4ca, "drm_mode_object_put" },
	{ 0x23f44b42, "drm_gem_object_lookup" },
	{ 0x9116c48d, "drm_crtc_send_vblank_event" },
	{ 0x53d29aec, "drm_connector_attach_encoder" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0xe8213e80, "_printk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x2520ea93, "refcount_warn_saturate" },
	{ 0x05e7ca86, "drm_atomic_helper_commit" },
	{ 0xd4a4112c, "put_device" },
	{ 0xe0c9e2f0, "drm_atomic_helper_check" },
	{ 0x97f1c754, "drm_atomic_helper_connector_destroy_state" },
	{ 0xd4a4112c, "root_device_unregister" },
	{ 0xe8ee094b, "drm_clflush_pages" },
	{ 0x9479a1e8, "strnlen" },
	{ 0x38c8be28, "idr_get_next" },
	{ 0x5947269e, "drm_gem_mmap" },
	{ 0x28f30db4, "drm_atomic_helper_plane_duplicate_state" },
	{ 0xcac6bbbd, "_dev_info" },
	{ 0x38129c24, "drm_gem_prime_import" },
	{ 0x4f423410, "drm_ioctl" },
	{ 0x9bb9323d, "drm_plane_cleanup" },
	{ 0x212d603c, "dma_buf_begin_cpu_access" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xd70733be, "sized_strscpy" },
	{ 0x29256d3a, "drm_gem_object_free" },
	{ 0x47886e07, "usb_unregister_notify" },
	{ 0xac3816a3, "drm_atomic_helper_dirtyfb" },
	{ 0xddc9594a, "drm_gem_vm_close" },
	{ 0x51ee696c, "vm_insert_page" },
	{ 0xddc9594a, "drm_gem_vm_open" },
	{ 0x88a6e0f4, "drm_dev_unplug" },
	{ 0x9a09c0dd, "drm_connector_init" },
	{ 0xcac6bbbd, "_dev_err" },
	{ 0x96845d77, "device_add" },
	{ 0x6936a446, "__root_device_register" },
	{ 0xa470b7e2, "add_timer" },
	{ 0x5a844b26, "__x86_indirect_thunk_r14" },
	{ 0x102ec640, "platform_device_unregister" },
	{ 0x040a4bb9, "drm_add_edid_modes" },
	{ 0x32feeafc, "mod_timer" },
	{ 0xc0112974, "noop_llseek" },
	{ 0x5f3f7934, "drm_read" },
	{ 0xa10c73d3, "dma_resv_fini" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xfae00f4b, "drm_gem_object_release" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0xd94efd11, "const_current_task" },
	{ 0xb3306581, "drm_crtc_vblank_on" },
	{ 0xef968900, "vmap" },
	{ 0x6b3607b2, "drm_gem_handle_create" },
	{ 0x704c679d, "drm_atomic_helper_plane_reset" },
	{ 0xc61f53f2, "drm_crtc_init_with_planes" },
	{ 0xac849640, "drm_mode_duplicate" },
	{ 0x57f11e28, "drm_helper_mode_fill_fb_struct" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x0155d982, "drm_edid_read_custom" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0x86d845a2, "__drm_dev_dbg" },
	{ 0xeccb097e, "drm_prime_pages_to_sg" },
	{ 0x81a1a811, "_raw_spin_unlock_irqrestore" },
	{ 0xd4a4112c, "device_del" },
	{ 0x5fc55113, "__default_kernel_pte_mask" },
	{ 0xbd858ef2, "__devm_drm_dev_alloc" },
	{ 0xcac6bbbd, "_dev_warn" },
	{ 0x77d32e97, "drm_connector_update_edid_property" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xdc292677, "drm_atomic_helper_set_config" },
	{ 0x386e4ba3, "kmemdup_noprof" },
	{ 0x765931eb, "dma_buf_vunmap" },
	{ 0x3bc3ef1e, "drm_atomic_helper_connector_duplicate_state" },
	{ 0x53740b6d, "drm_edid_raw" },
	{ 0x888b8f57, "strcmp" },
	{ 0x31e4086c, "drm_connector_cleanup" },
	{ 0x101f46af, "drm_get_format_info" },
	{ 0x058c185a, "jiffies" },
	{ 0xacbfeb0e, "__platform_driver_register" },
	{ 0xfae1fdbb, "drm_universal_plane_init" },
	{ 0xa175d115, "drm_atomic_helper_crtc_reset" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x82fd7238, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x23cee38e, "drm_encoder_init" },
	{ 0x2f9b0293, "platform_device_register_full" },
	{ 0xfc81f1ff, "drm_vblank_init" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xfae00f4b, "drm_gem_free_mmap_offset" },
	{ 0x13e72361, "filp_close" },
	{ 0xecd17989, "__kmalloc_cache_noprof" },
	{ 0xb3306581, "drm_crtc_vblank_off" },
	{ 0xda5f871c, "drm_helper_probe_single_connector_modes" },
	{ 0xd785d488, "drm_crtc_handle_vblank" },
	{ 0x02f9bbf0, "timer_init_key" },
	{ 0x04dde6dd, "dma_set_mask" },
	{ 0x286c8f41, "drm_atomic_helper_connector_reset" },
	{ 0x86f9c4c5, "drm_mode_config_reset" },
	{ 0xa7754093, "drm_format_info" },
	{ 0x47886e07, "usb_register_notify" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0xb3d1d601, "__kfifo_alloc" },
	{ 0x43a349ca, "strlen" },
	{ 0xf1de9e85, "kvfree" },
	{ 0x46d89baf, "__vma_start_write" },
	{ 0xdb2ba289, "drm_atomic_helper_crtc_duplicate_state" },
	{ 0xde338d9a, "_raw_spin_unlock" },
	{ 0xf191dd70, "drm_atomic_helper_update_plane" },
	{ 0x90bf627e, "__kvmalloc_node_noprof" },
	{ 0x734c4977, "drm_gem_put_pages" },
	{ 0x0c88a127, "drm_dev_register" },
	{ 0x646b83fb, "drm_gem_plane_helper_prepare_fb" },
	{ 0x08bfc903, "kmalloc_caches" },
	{ 0x829f6891, "drm_prime_gem_destroy" },
	{ 0x0f0f139f, "drm_release" },
	{ 0x1e96a996, "drm_gem_prime_export" },
	{ 0x814e12e5, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x1bdf2bc8,
	0xe4f8ce20,
	0xf12c0013,
	0x22755710,
	0x0f0f139f,
	0x6b720f69,
	0x8cede46e,
	0x553959eb,
	0x9f780415,
	0xd9a2bb0b,
	0x62ee94dc,
	0xfa552b22,
	0x9bb9323d,
	0x143526e8,
	0xea97e6f1,
	0x4d8419c6,
	0x5af83d53,
	0x212d603c,
	0x033456f6,
	0xa10c73d3,
	0xac823d72,
	0xf6ed07f0,
	0x59c5f726,
	0x30add0f2,
	0xd4a4112c,
	0x5c12d4ca,
	0xa53f4e29,
	0xf1de9e85,
	0xcb8b6ec6,
	0x1e419e08,
	0xc31b1c55,
	0x417f4070,
	0x2352b148,
	0x4c190cbe,
	0xa96ff906,
	0x283bce08,
	0x0eaef536,
	0xe1e1f979,
	0xde338d9a,
	0xd272d446,
	0x5c12d4ca,
	0x23f44b42,
	0x9116c48d,
	0x53d29aec,
	0x5a844b26,
	0xe8213e80,
	0xbd03ed67,
	0xd272d446,
	0x2520ea93,
	0x05e7ca86,
	0xd4a4112c,
	0xe0c9e2f0,
	0x97f1c754,
	0xd4a4112c,
	0xe8ee094b,
	0x9479a1e8,
	0x38c8be28,
	0x5947269e,
	0x28f30db4,
	0xcac6bbbd,
	0x38129c24,
	0x4f423410,
	0x9bb9323d,
	0x212d603c,
	0x90a48d82,
	0xd70733be,
	0x29256d3a,
	0x47886e07,
	0xac3816a3,
	0xddc9594a,
	0x51ee696c,
	0xddc9594a,
	0x88a6e0f4,
	0x9a09c0dd,
	0xcac6bbbd,
	0x96845d77,
	0x6936a446,
	0xa470b7e2,
	0x5a844b26,
	0x102ec640,
	0x040a4bb9,
	0x32feeafc,
	0xc0112974,
	0x5f3f7934,
	0xa10c73d3,
	0xbd03ed67,
	0xfae00f4b,
	0xf46d5bf3,
	0xd94efd11,
	0xb3306581,
	0xef968900,
	0x6b3607b2,
	0x704c679d,
	0xc61f53f2,
	0xac849640,
	0x57f11e28,
	0xc1e6c71e,
	0x0155d982,
	0xe54e0a6b,
	0x86d845a2,
	0xeccb097e,
	0x81a1a811,
	0xd4a4112c,
	0x5fc55113,
	0xbd858ef2,
	0xcac6bbbd,
	0x77d32e97,
	0xd272d446,
	0xdc292677,
	0x386e4ba3,
	0x765931eb,
	0x3bc3ef1e,
	0x53740b6d,
	0x888b8f57,
	0x31e4086c,
	0x101f46af,
	0x058c185a,
	0xacbfeb0e,
	0xfae1fdbb,
	0xa175d115,
	0xdd6830c7,
	0x82fd7238,
	0x23cee38e,
	0x2f9b0293,
	0xfc81f1ff,
	0xf46d5bf3,
	0xfae00f4b,
	0x13e72361,
	0xecd17989,
	0xb3306581,
	0xda5f871c,
	0xd785d488,
	0x02f9bbf0,
	0x04dde6dd,
	0x286c8f41,
	0x86f9c4c5,
	0xa7754093,
	0x47886e07,
	0xe4de56b4,
	0xb3d1d601,
	0x43a349ca,
	0xf1de9e85,
	0x46d89baf,
	0xdb2ba289,
	0xde338d9a,
	0xf191dd70,
	0x90bf627e,
	0x734c4977,
	0x0c88a127,
	0x646b83fb,
	0x08bfc903,
	0x829f6891,
	0x0f0f139f,
	0x1e96a996,
	0x814e12e5,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"sme_me_mask\0"
	"drm_framebuffer_cleanup\0"
	"drm_atomic_helper_plane_destroy_state\0"
	"filp_open\0"
	"drm_open\0"
	"drm_mode_vrefresh\0"
	"drm_poll\0"
	"__drm_err\0"
	"drm_gem_get_pages\0"
	"dma_buf_vmap\0"
	"drm_kms_helper_poll_init\0"
	"drm_framebuffer_init\0"
	"drm_plane_enable_fb_damage_clips\0"
	"dev_set_name\0"
	"platform_driver_unregister\0"
	"param_ops_ushort\0"
	"drm_atomic_add_affected_planes\0"
	"dma_buf_end_cpu_access\0"
	"drm_mode_probed_add\0"
	"dma_resv_init\0"
	"drm_atomic_helper_disable_plane\0"
	"__kernel_write\0"
	"drm_edid_free\0"
	"drm_prime_sg_to_page_array\0"
	"device_initialize\0"
	"drm_mode_object_get\0"
	"memcpy\0"
	"vunmap\0"
	"kfree\0"
	"drm_atomic_helper_crtc_destroy_state\0"
	"drm_crtc_cleanup\0"
	"drm_gem_create_mmap_offset\0"
	"timer_delete\0"
	"drm_encoder_cleanup\0"
	"drm_gem_object_init\0"
	"__kfifo_free\0"
	"drmm_mode_config_init\0"
	"_raw_spin_lock_irqsave\0"
	"_raw_spin_lock\0"
	"__fentry__\0"
	"drm_mode_object_put\0"
	"drm_gem_object_lookup\0"
	"drm_crtc_send_vblank_event\0"
	"drm_connector_attach_encoder\0"
	"__x86_indirect_thunk_rax\0"
	"_printk\0"
	"__ref_stack_chk_guard\0"
	"__stack_chk_fail\0"
	"refcount_warn_saturate\0"
	"drm_atomic_helper_commit\0"
	"put_device\0"
	"drm_atomic_helper_check\0"
	"drm_atomic_helper_connector_destroy_state\0"
	"root_device_unregister\0"
	"drm_clflush_pages\0"
	"strnlen\0"
	"idr_get_next\0"
	"drm_gem_mmap\0"
	"drm_atomic_helper_plane_duplicate_state\0"
	"_dev_info\0"
	"drm_gem_prime_import\0"
	"drm_ioctl\0"
	"drm_plane_cleanup\0"
	"dma_buf_begin_cpu_access\0"
	"__ubsan_handle_out_of_bounds\0"
	"sized_strscpy\0"
	"drm_gem_object_free\0"
	"usb_unregister_notify\0"
	"drm_atomic_helper_dirtyfb\0"
	"drm_gem_vm_close\0"
	"vm_insert_page\0"
	"drm_gem_vm_open\0"
	"drm_dev_unplug\0"
	"drm_connector_init\0"
	"_dev_err\0"
	"device_add\0"
	"__root_device_register\0"
	"add_timer\0"
	"__x86_indirect_thunk_r14\0"
	"platform_device_unregister\0"
	"drm_add_edid_modes\0"
	"mod_timer\0"
	"noop_llseek\0"
	"drm_read\0"
	"dma_resv_fini\0"
	"random_kmalloc_seed\0"
	"drm_gem_object_release\0"
	"mutex_lock\0"
	"const_current_task\0"
	"drm_crtc_vblank_on\0"
	"vmap\0"
	"drm_gem_handle_create\0"
	"drm_atomic_helper_plane_reset\0"
	"drm_crtc_init_with_planes\0"
	"drm_mode_duplicate\0"
	"drm_helper_mode_fill_fb_struct\0"
	"__mutex_init\0"
	"drm_edid_read_custom\0"
	"__fortify_panic\0"
	"__drm_dev_dbg\0"
	"drm_prime_pages_to_sg\0"
	"_raw_spin_unlock_irqrestore\0"
	"device_del\0"
	"__default_kernel_pte_mask\0"
	"__devm_drm_dev_alloc\0"
	"_dev_warn\0"
	"drm_connector_update_edid_property\0"
	"__x86_return_thunk\0"
	"drm_atomic_helper_set_config\0"
	"kmemdup_noprof\0"
	"dma_buf_vunmap\0"
	"drm_atomic_helper_connector_duplicate_state\0"
	"drm_edid_raw\0"
	"strcmp\0"
	"drm_connector_cleanup\0"
	"drm_get_format_info\0"
	"jiffies\0"
	"__platform_driver_register\0"
	"drm_universal_plane_init\0"
	"drm_atomic_helper_crtc_reset\0"
	"sprintf\0"
	"__ubsan_handle_shift_out_of_bounds\0"
	"drm_encoder_init\0"
	"platform_device_register_full\0"
	"drm_vblank_init\0"
	"mutex_unlock\0"
	"drm_gem_free_mmap_offset\0"
	"filp_close\0"
	"__kmalloc_cache_noprof\0"
	"drm_crtc_vblank_off\0"
	"drm_helper_probe_single_connector_modes\0"
	"drm_crtc_handle_vblank\0"
	"timer_init_key\0"
	"dma_set_mask\0"
	"drm_atomic_helper_connector_reset\0"
	"drm_mode_config_reset\0"
	"drm_format_info\0"
	"usb_register_notify\0"
	"__ubsan_handle_load_invalid_value\0"
	"__kfifo_alloc\0"
	"strlen\0"
	"kvfree\0"
	"__vma_start_write\0"
	"drm_atomic_helper_crtc_duplicate_state\0"
	"_raw_spin_unlock\0"
	"drm_atomic_helper_update_plane\0"
	"__kvmalloc_node_noprof\0"
	"drm_gem_put_pages\0"
	"drm_dev_register\0"
	"drm_gem_plane_helper_prepare_fb\0"
	"kmalloc_caches\0"
	"drm_prime_gem_destroy\0"
	"drm_release\0"
	"drm_gem_prime_export\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0CDD45319144BEDBDC888D7");
