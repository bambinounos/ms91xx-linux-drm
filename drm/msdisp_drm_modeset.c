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
 * msdisp_drm_modeset.c -- Drm driver for MacroSilicon chip 913x and 912x
 */


#include <linux/version.h>
#if KERNEL_VERSION(5, 16, 0) <= LINUX_VERSION_CODE
#include <drm/drm_vblank.h>
#include <drm/drm_damage_helper.h>
#elif KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE || defined(EL8)
#include <drm/drm_damage_helper.h>
#else
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_atomic_helper.h>
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
#include <drm/drm_gem_atomic_helper.h>
#else
#include <drm/drm_gem_framebuffer_helper.h>
#endif

#include <linux/dma-buf.h>


#include "msdisp_drm_drv.h"
#include "msdisp_drm_event.h"
#include "msdisp_common_util.h"
#include "msdisp_usb_interface.h"


static struct msdisp_drm_pipeline* get_pipeline_by_plane(struct drm_plane* plane)
{
	struct msdisp_drm_device *msdisp_drm = to_msdisp_drm(plane->dev);
	struct msdisp_drm_pipeline* pipeline = NULL;
	int i;

	for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
		if (msdisp_drm->pipeline[i].crtc->primary == plane) {
			pipeline = &msdisp_drm->pipeline[i];
			break;
		}
	}

	return pipeline;
}

static struct msdisp_drm_pipeline* get_pipeline_by_crtc(struct drm_crtc* crtc)
{
	struct msdisp_drm_device *msdisp_drm = to_msdisp_drm(crtc->dev);
	struct msdisp_drm_pipeline* pipeline = NULL;
	int i;

	for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
		if (msdisp_drm->pipeline[i].crtc == crtc) {
			pipeline = &msdisp_drm->pipeline[i];
			break;
		}
	}

	return pipeline;
}

void msdisp_crtc_update_event(struct drm_crtc *crtc)
{
	struct drm_device* dev = crtc->dev;
	struct msdisp_drm_pipeline* pipeline = get_pipeline_by_crtc(crtc);

	if (crtc->state->event) {
		unsigned long flags;

		crtc->state->event->pipe = drm_crtc_index(crtc);
		spin_lock_irqsave(&dev->event_lock, flags);
		pipeline->event = crtc->state->event;
		crtc->state->event = NULL;
		spin_unlock_irqrestore(&dev->event_lock, flags);
	}
}


static void msdisp_drm_crtc_destroy(struct drm_crtc *crtc)
{
	drm_crtc_cleanup(crtc);
	kfree(crtc);
}

void msdisp_drm_crtc_atomic_flush(struct drm_crtc *crtc, 
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_atomic_state *state
#else
	struct drm_crtc_state *old_state
#endif
)
{
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_crtc_state *crtc_state = drm_atomic_get_old_crtc_state(state, crtc);
#else
	struct drm_crtc_state *crtc_state = old_state;
#endif

	if (crtc->state->active && crtc_state->active){
		msdisp_crtc_update_event(crtc);
	}
}

int msdisp_drm_crtc_atomic_check(struct drm_crtc *crtc, 
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_atomic_state *state
#else
	struct drm_crtc_state *new_state
#endif
)
{
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_crtc_state *crtc_state = drm_atomic_get_new_crtc_state(state, crtc);
#else
	struct drm_crtc_state *crtc_state = new_state;
#endif

	bool has_primary = (crtc_state->plane_mask & drm_plane_mask(crtc->primary));

	/* We always want to have an active plane with an active CRTC */
	if (has_primary != crtc_state->enable)
		return -EINVAL;

	return drm_atomic_add_affected_planes(crtc_state->state, crtc);
}

void msdisp_drm_crtc_atomic_enable(struct drm_crtc *crtc, 
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_atomic_state *state
#else
	struct drm_crtc_state *old_crtc_state
#endif
)
{
	struct drm_device *dev = crtc->dev;
	struct drm_plane *primary_plane = crtc->primary;
	struct drm_plane_state *plane_state = primary_plane->state;
	struct drm_framebuffer *fb = plane_state->fb;
	struct drm_crtc_state *crtc_state = crtc->state;
	struct drm_display_mode *mode = &crtc_state->mode;
	struct msdisp_drm_pipeline* pipeline;
	struct msdisp_usb_hal* usb_hal;
    int width, height, rate;

	drm_crtc_vblank_on(crtc);
	msdisp_crtc_update_event(crtc);

    dev_info(dev->dev, "enable: pid=%d comm=%s\n", task_pid_nr(current), current->comm);
	dev_info(dev->dev, "fb size:width:%d height:%d mode size:width:%d height:%d\n", fb->width, fb->height, mode->hdisplay, mode->vdisplay);
	pipeline = get_pipeline_by_crtc(crtc);
	mutex_lock(&pipeline->hal_lock);
	usb_hal = pipeline->usb_hal;
	if (!usb_hal) {
		dev_info(dev->dev, "usb hal is null\n");
		goto out;
	}

    width = mode->hdisplay;
    height = mode->vdisplay;
	rate = drm_mode_vrefresh(mode);

	pipeline->drm_width = width;
	pipeline->drm_height = height;
	pipeline->drm_rate = rate;
	pipeline->drm_fb_format = fb->format->format;
	pipeline->drm_status = MSDISP_DRM_STATUS_ENABLE;

    usb_hal->funcs->enable(usb_hal, width, height, rate, fb->format->format);
    dev_info(dev->dev, "enable event:format=0x%x width=%d height=%d rate=%d\n", fb->format->format, width, height, rate);
out:
	mutex_unlock(&pipeline->hal_lock);
}

void msdisp_drm_crtc_atomic_disable(struct drm_crtc *crtc, 
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	struct drm_atomic_state *state
#else
	struct drm_crtc_state *old_crtc_state
#endif
)
{
	struct drm_device *dev = crtc->dev;
	struct msdisp_drm_pipeline* pipeline;
	struct msdisp_usb_hal* usb_hal;

	
	drm_crtc_vblank_off(crtc);
	if (crtc->state->event) {
		unsigned long flags;

		spin_lock_irqsave(&dev->event_lock, flags);
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
		spin_unlock_irqrestore(&dev->event_lock, flags);
	}
	pipeline = get_pipeline_by_crtc(crtc);
	pipeline->drm_status = MSDISP_DRM_STATUS_DISABLE;
    dev_info(dev->dev, "disable: pid=%d! comm=%s\n", task_pid_nr(current), current->comm);
	mutex_lock(&pipeline->hal_lock);
	usb_hal = pipeline->usb_hal;

	if (!usb_hal) {
		dev_info(dev->dev, "usb hal is null\n");
		goto out;
	}

    usb_hal->funcs->disable(usb_hal);
out:
	mutex_unlock(&pipeline->hal_lock);
}

static struct drm_crtc_helper_funcs msdisp_drm_helper_funcs = {
	.atomic_check = msdisp_drm_crtc_atomic_check,
	.atomic_enable = msdisp_drm_crtc_atomic_enable,
	.atomic_disable = msdisp_drm_crtc_atomic_disable,
	.atomic_flush = msdisp_drm_crtc_atomic_flush,
};

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
static int msdisp_drm_enable_vblank(struct drm_crtc *crtc)
{
	return 0;
}

static void msdisp_drm_disable_vblank(struct drm_crtc *crtc)
{

}
#endif


static int
msdisp_crtc_cursor_set(struct drm_crtc *crtc, struct drm_file *file_priv,
		     uint32_t buffer_handle, uint32_t width, uint32_t height)
{
	int ret = 0;
	void *vmapping;
	struct msdisp_usb_hal* usb_hal;
	struct drm_gem_object *obj;
	struct page **pages;
	struct msdisp_drm_pipeline* pipeline = get_pipeline_by_crtc(crtc);

	if (!pipeline) {
		//printk("msdisp: set cursor no pipeline.\n");
		return ret;
	}

	usb_hal = pipeline->usb_hal;
	if (!usb_hal) {
		//printk("msdisp: set cursor usb_hal is null.\n");
		return ret;
	}
	if (buffer_handle == 0) {
		usb_hal->funcs->cursor_set(usb_hal, NULL);
		return 0;
	}

	/* Currently we only support 64x64 cursors */
	if (width != 64 || height != 64) {
		printk("We currently only support 64x64 cursors :%dx%d\n", width, height);
		return -EINVAL;
	}

	obj = drm_gem_object_lookup(file_priv, buffer_handle);
	if (!obj) {
		goto unlock;
	}

	if (obj->size == 0 || obj->size < width * height * 4) {
		printk("Buffer is too small\n");
		goto unlock;
	}

	pages = drm_gem_get_pages(obj);
	vmapping = vmap(pages, 4, 0, PAGE_KERNEL);

	usb_hal->funcs->cursor_set(usb_hal, vmapping);
	vunmap(vmapping);
	kvfree(pages);

unlock:
	return ret;
}

static int
msdisp_crtc_cursor_move(struct drm_crtc *crtc, int x, int y)
{
	struct msdisp_drm_pipeline* pipeline = get_pipeline_by_crtc(crtc);
	struct msdisp_usb_hal* usb_hal;

	if (!pipeline) {
		return 0;
	}

	usb_hal = pipeline->usb_hal;
	if (!usb_hal) {
		return 0;
	}

	usb_hal->funcs->cursor_move(usb_hal, x, y);

	return 0;
}

static const struct drm_crtc_funcs msdisp_drm_crtc_funcs = {
	.reset                  = drm_atomic_helper_crtc_reset,
	.destroy                = msdisp_drm_crtc_destroy,
	.set_config             = drm_atomic_helper_set_config,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state   = drm_atomic_helper_crtc_destroy_state,
	.cursor_set             = msdisp_crtc_cursor_set,
	.cursor_move            = msdisp_crtc_cursor_move,
#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE || defined(RPI) || defined(EL8)
	.enable_vblank          = msdisp_drm_enable_vblank,
	.disable_vblank         = msdisp_drm_disable_vblank,
#endif
};

static int msdisp_drm_handle_damage(struct msdisp_drm_framebuffer *efb, struct msdisp_drm_pipeline *pipeline)
{
	struct drm_framebuffer* fb = &efb->base;
	struct msdisp_usb_hal* usb_hal = pipeline->usb_hal;
	u8* src;
	int len;

	src = (u8*)(efb->obj->vmapping);
	len = fb->pitches[0] * fb->height;

	return usb_hal->funcs->update_frame(usb_hal, src, fb->pitches[0], len, fb->format->format, 0);
}

static void msdisp_drm_plane_atomic_update(struct drm_plane *plane,
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
				     struct drm_atomic_state *atom_state
#else
				     struct drm_plane_state *old_state
#endif
		)
{
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
	struct drm_plane_state *old_state = drm_atomic_get_old_plane_state(atom_state, plane);
#else
#endif
	struct drm_device* dev;
	struct msdisp_drm_device *msdisp_drm;
	struct msdisp_usb_hal* usb_hal;
	struct drm_framebuffer *fb;
	struct msdisp_drm_framebuffer *efb;
	struct dma_buf_attachment *import_attach;
	struct msdisp_drm_frame_stat* stat;
	struct msdisp_drm_pipeline* pipeline;
	int ret, lock_flag = 0;

	if (!plane || !plane->state || !plane->dev) {
		return;
	}

	dev = plane->dev;

	pipeline = get_pipeline_by_plane(plane);
	stat = &pipeline->frame_stat;
	stat->total++;

	if (!old_state) {
		stat->no_old_state++;
		return;
	}

	fb = old_state->fb;
	if (!fb) {
		stat->no_fb++;
		return;
	}

	efb = to_msdisp_drm_fb(fb);

	drm_framebuffer_get(&efb->base);

	if (!efb->obj->vmapping) {
		if (msdisp_drm_gem_vmap(efb->obj) == -ENOMEM) {
			dev_err(dev->dev, "Failed to map scanout buffer\n");
			stat->vmap_fail++;
			goto err_fb;
		}
		if (!efb->obj->vmapping) {
			dev_err(dev->dev, "Vmapping does not exists!\n");
			stat->vmap_null++;
			goto err_fb;
		}
	}

	import_attach = efb->obj->base.import_attach;
	if (import_attach) {
		ret = dma_buf_begin_cpu_access(import_attach->dmabuf,
					       DMA_FROM_DEVICE);
		if (ret) {
			dev_err(dev->dev, "dma_buf begin cpu access failed! ret=%d!\n", ret);
			stat->cpu_access_fail++;
			goto err_fb;
		}
	}

	if (pipeline->dump_fb_flag) {
        msdisp_common_save_buf_to_bmp(efb->obj->vmapping, fb->width, fb->height, fb->format->cpp[0], NULL, pipeline->dump_fb_filename);
        pipeline->dump_fb_flag = 0;
        dev_info(msdisp_drm->drm.dev, "msdisp finished save raw fb data to file:%s\n", pipeline->dump_fb_filename);
    }

	mutex_lock(&pipeline->hal_lock);
	lock_flag = 1;
	usb_hal = pipeline->usb_hal;
	if (!pipeline->usb_hal) {
		stat->no_usb_hal++;
		goto end_cpu_access;
	}

	if (msdisp_drm_handle_damage(efb, pipeline)) {
		stat->handle_fail++;
	}

end_cpu_access:
	if (import_attach)
		dma_buf_end_cpu_access(import_attach->dmabuf,
				       DMA_FROM_DEVICE);

err_fb:
	drm_framebuffer_put(&efb->base);

	if (lock_flag) {
		mutex_unlock(&pipeline->hal_lock);
	}
}

static const struct drm_plane_helper_funcs msdisp_drm_plane_helper_funcs = {
	.atomic_update = msdisp_drm_plane_atomic_update,
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
	.prepare_fb = drm_gem_plane_helper_prepare_fb
#else
	.prepare_fb = drm_gem_fb_prepare_fb
#endif
};

static const struct drm_plane_funcs msdisp_drm_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = drm_plane_cleanup,
	.reset = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};

static const uint32_t formats[] = {
	DRM_FORMAT_XRGB8888,
};

static struct drm_plane *msdisp_drm_create_plane(
		struct drm_device *dev,
		enum drm_plane_type type,
		const struct drm_plane_helper_funcs *helper_funcs)
{
	struct drm_plane *plane;
	int ret;
	char *plane_type = (type == DRM_PLANE_TYPE_CURSOR) ? "cursor" : "primary";

	plane = kzalloc(sizeof(*plane), GFP_KERNEL);
	if (plane == NULL) {
		dev_err(dev->dev, "Failed to allocate %s plane\n", plane_type);
		return NULL;
	}
	plane->format_default = true;

	ret = drm_universal_plane_init(dev,
				       plane,
				       0xFF,
				       &msdisp_drm_plane_funcs,
				       formats,
				       ARRAY_SIZE(formats),
				       NULL,
				       type,
				       plane_type
				       );

	if (ret) {
		dev_err(dev->dev, "Failed to initialize %s plane\n", plane_type);
		kfree(plane);
		return NULL;
	}

	drm_plane_helper_add(plane, helper_funcs);

	return plane;
}

static struct drm_crtc* msdisp_drm_crtc_init(struct drm_device *dev)
{
	struct drm_crtc* crtc = NULL;
	struct drm_plane *primary_plane = NULL;
	struct drm_plane *cursor_plane = NULL;
	int status = 0;

	crtc = kzalloc(sizeof(struct drm_crtc), GFP_KERNEL);
	if (!crtc) {
		return NULL;
	}

	primary_plane = msdisp_drm_create_plane(dev, DRM_PLANE_TYPE_PRIMARY,
					  &msdisp_drm_plane_helper_funcs);

	if (!primary_plane) {
		return NULL;
	}

#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE || defined(EL8)
	drm_plane_enable_fb_damage_clips(primary_plane);
#endif

	status = drm_crtc_init_with_planes(dev, crtc,
				primary_plane, cursor_plane,
				&msdisp_drm_crtc_funcs,
				NULL
				);

	drm_crtc_helper_add(crtc, &msdisp_drm_helper_funcs);

	return crtc;
}

static const struct drm_mode_config_funcs msdisp_drm_mode_funcs = {
	.fb_create = msdisp_drm_fb_user_fb_create,
	.atomic_commit = drm_atomic_helper_commit,
	.atomic_check = drm_atomic_helper_check
};

int msdisp_drm_modeset_init(struct drm_device *dev)
{
	struct msdisp_drm_device *msdisp_drm = to_msdisp_drm(dev);
	struct drm_crtc* crtc;
	struct msdisp_drm_connector *connector;
    struct drm_encoder *encoder;
	int i, pipeline_cnt;

	drm_mode_config_init(dev);

	dev->mode_config.min_width = 640;
	dev->mode_config.min_height = 480;
	dev->mode_config.max_width = 1920;
	dev->mode_config.max_height = 1600;
	dev->mode_config.cursor_width = 64;
	dev->mode_config.cursor_height = 64;
	dev->mode_config.prefer_shadow = 0;
	dev->mode_config.preferred_depth = 32;
	//dev->mode_config.preferred_depth = 16;

	dev->mode_config.funcs = &msdisp_drm_mode_funcs;

	pipeline_cnt = msdisp_drm_get_pipeline_init_count();
	for (i = 0; i < pipeline_cnt; i++) {
		crtc = msdisp_drm_crtc_init(dev);
		if (!crtc) {
			dev_err(dev->dev, "Failed to init crtc%d\n", i);
			goto err;
		}

		encoder = msdisp_drm_encoder_init(dev);
		if (!encoder) {
			dev_err(dev->dev, "Failed to init encoder%d\n", i);
			goto err;
		}
		encoder->possible_crtcs = (1 << i);

		connector = msdisp_drm_connector_init(dev, encoder, i);
		if (!connector) {
			dev_err(dev->dev, "Failed to init connector%d\n", i);
			goto err;
		}
		

		msdisp_drm->pipeline[i].crtc = crtc;
		msdisp_drm->pipeline[i].encoder = encoder;
		msdisp_drm->pipeline[i].connector = connector;

		continue;
err:
		if (crtc) {
			kfree(crtc);
		}

		if (encoder) {
			kfree(encoder);
		}

		if (connector) {
			kfree(connector);
		}

		return -ENOMEM;
	}
	drm_mode_config_reset(dev);

	return 0;
}
