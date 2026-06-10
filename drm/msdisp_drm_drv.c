/* Copyright (C) 2023 MacroSilicon Technology Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * msdisp_drm_drv.c -- Drm driver for MacroSilicon chip 913x and 912x
 */

#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#if KERNEL_VERSION(5, 16, 0) <= LINUX_VERSION_CODE
#include <drm/drm_ioctl.h>
#include <drm/drm_file.h>
#include <drm/drm_drv.h>
#include <drm/drm_vblank.h>
#elif KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
#include <drm/drmP.h>
#endif
#if KERNEL_VERSION(5, 1, 0) <= LINUX_VERSION_CODE || defined(EL8)
#include <drm/drm_probe_helper.h>
#endif
#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
#include <drm/drm_managed.h>
#endif
#include <drm/drm_atomic_helper.h>


#include "msdisp_drm_drv.h"
#include "msdisp_plat_drv.h"

#define	MSDISP_DRM_VBLANK_TIMER_OUT_MS				20

static ushort msdisp_drm_initial_pipeline_count = 3;
module_param_named(initial_pipeline_count,
		   msdisp_drm_initial_pipeline_count, ushort, 0644);
MODULE_PARM_DESC(initial_pipeline_count, "Initial DRM device pipeline counts (default: 3)");

int msdisp_drm_get_pipeline_init_count(void)
{
	return msdisp_drm_initial_pipeline_count;
}


void msdisp_drm_sysfs_init(struct msdisp_drm_device * msdisp_drm);
void msdisp_drm_sysfs_exit(struct msdisp_drm_device * msdisp_drm);

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
static const struct vm_operations_struct msdisp_drm_gem_vm_ops = {
	.fault = msdisp_drm_gem_fault,
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};
#endif

static const struct file_operations msdisp_drm_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.mmap = msdisp_drm_gem_mmap,
	.poll = drm_poll,
	.read = drm_read,
	.unlocked_ioctl = drm_ioctl,
	.release = drm_release,
	.llseek = noop_llseek,
};

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
static int msdisp_drm_enable_vblank(struct drm_device *dev, unsigned int pipe)
{
	return 0;
}

static void msdisp_drm_disable_vblank(struct drm_device *dev, unsigned int pipe)
{

}
#endif

static struct drm_driver driver = {
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE || defined(EL8)
	.driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC,
#else
	.driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_PRIME
			 | DRIVER_ATOMIC,
#endif

	/* gem hooks */
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(EL8)
#elif KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	.gem_free_object_unlocked = msdisp_drm_gem_free_object,
#else
	.gem_free_object = msdisp_drm_gem_free_object,
#endif

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
	.gem_vm_ops = &msdisp_drm_gem_vm_ops,
#endif

	.dumb_create = msdisp_drm_dumb_create,
	.dumb_map_offset = msdisp_drm_gem_mmap_offset,
#if KERNEL_VERSION(5, 12, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
	.dumb_destroy = drm_gem_dumb_destroy,
#endif

	.fops = &msdisp_drm_driver_fops,

	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_import = drm_gem_prime_import,
	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(EL8)
#else
	.gem_prime_export = drm_gem_prime_export,
	.gem_prime_get_sg_table = msdisp_drm_prime_get_sg_table,
	.enable_vblank = msdisp_drm_enable_vblank,
	.disable_vblank = msdisp_drm_disable_vblank,
#endif
	.gem_prime_import_sg_table = msdisp_drm_prime_import_sg_table,

	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCH,
};

static void msdisp_drm_handle_page_flip(struct msdisp_drm_pipeline* pipeline)
{
	struct drm_crtc* crtc = pipeline->crtc;
	struct drm_device* dev = crtc->dev;
	unsigned long flags;

	spin_lock_irqsave(&dev->event_lock, flags);
	if (pipeline->event) {
		drm_crtc_send_vblank_event(crtc, pipeline->event);
		pipeline->event = NULL;
	}
	spin_unlock_irqrestore(&dev->event_lock, flags);
}

static void msidsip_drm_timer_func(struct timer_list* t)
{
	struct msdisp_drm_device *msdisp = from_timer(msdisp, t, vblank_timer); 
	struct drm_crtc* crtc;
	int i;

	for (i = 0; i < msdisp->pipeline_cnt; i++) {
		crtc = msdisp->pipeline[i].crtc;
		drm_crtc_handle_vblank(crtc);
		msdisp_drm_handle_page_flip(&msdisp->pipeline[i]);
	}

	mod_timer(&msdisp->vblank_timer, jiffies + msecs_to_jiffies(MSDISP_DRM_VBLANK_TIMER_OUT_MS));
}

static int msdisp_drm_init(struct msdisp_drm_device *msdisp)
{
	struct drm_device *dev = &msdisp->drm;
	int i;
	int ret = -ENOMEM;

	for (i = 0; i < msdisp->pipeline_cnt; i++) {
		msdisp->pipeline[i].drm_status = MSDISP_DRM_STATUS_DISABLE;
		mutex_init(&msdisp->pipeline[i].hal_lock);
	}

	timer_setup(&msdisp->vblank_timer, msidsip_drm_timer_func, 0);
	msdisp->vblank_timer.expires = (jiffies + msecs_to_jiffies(MSDISP_DRM_VBLANK_TIMER_OUT_MS));
	add_timer(&msdisp->vblank_timer);
 
	ret = msdisp_drm_modeset_init(dev);
	if (ret) {
		goto err;
	}

#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	dev->vblank_disable_immediate = true;
#else
#if KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	dev->irq_enabled = true;
#endif
#else
	dev->irq_enabled = true;
#endif
#endif
    ret = drm_vblank_init(dev, msdisp->pipeline_cnt);
	if (ret)
		goto err;

	drm_kms_helper_poll_init(dev);

err:
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
#else
/* added by Fuzhao Jin */
static void devm_drm_dev_init_release(void *data)
{
	drm_dev_put(data);
}

int devm_drm_dev_init(struct device *parent,
		      struct drm_device *dev,
		      struct drm_driver *driver)
{
	int ret;

	ret = drm_dev_init(dev, driver, parent);
	if (ret)
		return ret;

	ret = devm_add_action(parent, devm_drm_dev_init_release, dev);
	if (ret)
		devm_drm_dev_init_release(dev);

	return ret;
}

void *__devm_drm_dev_alloc(struct device *parent, struct drm_driver *driver,
			   size_t size, size_t offset)
{
	void *container;
	struct drm_device *drm;
	int ret;

	container = kzalloc(size, GFP_KERNEL);
	if (!container)
		return ERR_PTR(-ENOMEM);

	drm = container + offset;
	ret = devm_drm_dev_init(parent, drm, driver);
	if (ret) {
		kfree(container);
		return ERR_PTR(ret);
	}

	return container;
}

#define devm_drm_dev_alloc(parent, driver, type, member) \
	((type *) __devm_drm_dev_alloc(parent, driver, sizeof(type), \
				       offsetof(type, member)))
#endif

struct drm_device *msdisp_drm_device_create(struct device *parent)
{
    struct msdisp_drm_device *msdisp_drm = NULL;
	struct drm_device *drm = NULL;
	int ret, i, alloc_fifo_cnt = 0;

	if (msdisp_drm_initial_pipeline_count > MSDISP_DRM_MAX_PIPELINE_CNT) {
		printk("%s: Max pipeline is :%d module param is:%d\n", __func__, MSDISP_DRM_MAX_PIPELINE_CNT, msdisp_drm_initial_pipeline_count);
		return NULL;
	}

	msdisp_drm = devm_drm_dev_alloc(parent, &driver, struct msdisp_drm_device, drm);

    if(NULL == msdisp_drm) {
        dev_err(parent, "alloc msdisp drm device failed!\n");
        return NULL;
    }

    drm = &msdisp_drm->drm;
	msdisp_drm->pipeline_cnt = msdisp_drm_initial_pipeline_count;


	for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
		ret = kfifo_alloc(&msdisp_drm->pipeline[i].fifo, 1024, GFP_KERNEL);
		if (ret) {
			dev_err(drm->dev, "alloc kfifo%d failed!\n ret = %d\n", i, ret);
			ret = -ENOMEM;
			goto err_free;
		}
		alloc_fifo_cnt = i;
	}
	
	ret = msdisp_drm_init(msdisp_drm);
	if (ret)
		goto err_free;


	ret = drm_dev_register(drm, 0);
	if (ret)
		goto err_free;

	msdisp_drm_sysfs_init(msdisp_drm);
	return drm;

err_free:
	for (i = 0; i < alloc_fifo_cnt; i++) {
		(void)kfifo_free(&msdisp_drm->pipeline[i].fifo);
	}
	return ERR_PTR(ret);
}

int msdisp_drm_device_remove(struct drm_device *drm)
{
	int i;
	struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);

	for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
		(void)kfifo_free(&msdisp_drm->pipeline[i].fifo);
	}

	del_timer(&msdisp_drm->vblank_timer);
	msdisp_drm_sysfs_exit(msdisp_drm);
	drm_dev_unplug(drm);

	return 0;
}