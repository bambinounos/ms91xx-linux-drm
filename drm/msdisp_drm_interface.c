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
 * msdisp_drm_interface.c -- Drm driver for MacroSilicon chip 913x and 912x
 */


#include <linux/version.h>
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
#include <linux/platform_device.h>


#include "msdisp_drm_drv.h"
#include "msdisp_drm_connector.h"
#include "msdisp_plat_drv.h"
#include "msdisp_drm_interface.h"
#include "msdisp_drm_event.h"
#include "msdisp_usb_interface.h"


static int has_free_pipeline(struct msdisp_drm_device* msdisp_drm)
{
    int i;
    int ret = 0;

    mutex_lock(&msdisp_drm->pipeline_lock);
    for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
        if (!msdisp_drm->pipeline[i].occupied && !msdisp_drm->pipeline[i].reg_flag) {
            ret = 1;
            break;
        }
    }
    mutex_unlock(&msdisp_drm->pipeline_lock);

    return ret;
}

struct drm_device* msdisp_drm_get_free_device(void)
{
    int i;
    struct platform_device* pdev;
    struct drm_device* drm;
    struct msdisp_drm_device* msdisp_drm;

    for (i = 0; i < MSDISP_DEVICE_COUNT_MAX; i++) {
         pdev = msdisp_platform_get_device(i);
         if (!pdev) {
            continue;
         }
         drm = platform_get_drvdata(pdev);
         if (!drm) {
            continue;
         }
         msdisp_drm = to_msdisp_drm(drm);
         if (has_free_pipeline(msdisp_drm)) {
            return drm;
         }
    }
    return NULL;
}
EXPORT_SYMBOL(msdisp_drm_get_free_device);

int msdisp_drm_get_drm_device_index(struct drm_device* drm)
{
    int i;
    struct platform_device* pdev;
    struct drm_device* plat_drm;

    if (!drm) {
        return -1;
    }

    for (i = 0; i < MSDISP_DEVICE_COUNT_MAX; i++) {
         pdev = msdisp_platform_get_device(i);
         if (!pdev) {
            continue;
         }
         plat_drm = platform_get_drvdata(pdev);
         if (!plat_drm) {
            continue;
         }
         
         if (plat_drm == drm) {
            return i;
         }
         
    }
    return -1;
}
EXPORT_SYMBOL(msdisp_drm_get_drm_device_index);

int msdisp_drm_get_free_pipeline_index(struct drm_device* drm)
{
    int i;
    int index = -1;
    struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);

    mutex_lock(&msdisp_drm->pipeline_lock);
    for (i = 0; i < msdisp_drm->pipeline_cnt; i++) {
        if (!msdisp_drm->pipeline[i].occupied && !msdisp_drm->pipeline[i].reg_flag) {
            index = i;
            break;
        }
    }
    mutex_unlock(&msdisp_drm->pipeline_lock);

    return index;
}
EXPORT_SYMBOL(msdisp_drm_get_free_pipeline_index);

/* Atomically locate a DRM device with a free pipeline, reserve that pipeline
 * slot, and reset its event kfifo so concurrent USB probes (e.g. multiple
 * MS91xx adapters plugged in at boot or on a hub) never claim the same pipe. */
int msdisp_drm_acquire_free_pipeline(struct drm_device** out_drm)
{
    int i, p;
    struct platform_device* pdev;
    struct drm_device* drm;
    struct msdisp_drm_device* msdisp_drm;

    if (!out_drm) {
        return -EINVAL;
    }
    *out_drm = NULL;

    for (i = 0; i < MSDISP_DEVICE_COUNT_MAX; i++) {
        pdev = msdisp_platform_get_device(i);
        if (!pdev) {
            continue;
        }
        drm = platform_get_drvdata(pdev);
        if (!drm) {
            continue;
        }
        msdisp_drm = to_msdisp_drm(drm);
        mutex_lock(&msdisp_drm->pipeline_lock);
        for (p = 0; p < msdisp_drm->pipeline_cnt; p++) {
            if (!msdisp_drm->pipeline[p].occupied && !msdisp_drm->pipeline[p].reg_flag) {
                msdisp_drm->pipeline[p].occupied = 1;
                kfifo_reset(&msdisp_drm->pipeline[p].fifo);
                mutex_unlock(&msdisp_drm->pipeline_lock);
                *out_drm = drm;
                return p;
            }
        }
        mutex_unlock(&msdisp_drm->pipeline_lock);
    }

    return -ENODEV;
}
EXPORT_SYMBOL(msdisp_drm_acquire_free_pipeline);

void msdisp_drm_release_pipeline(struct drm_device* drm, int pipeline_index)
{
    struct msdisp_drm_device* msdisp_drm;
    struct msdisp_drm_pipeline* pipeline;

    if (!drm || pipeline_index < 0 || pipeline_index >= MSDISP_DRM_MAX_PIPELINE_CNT) {
        return;
    }

    msdisp_drm = to_msdisp_drm(drm);
    if (pipeline_index >= msdisp_drm->pipeline_cnt) {
        return;
    }

    pipeline = &msdisp_drm->pipeline[pipeline_index];
    mutex_lock(&msdisp_drm->pipeline_lock);
    kfifo_reset(&pipeline->fifo);
    pipeline->reg_flag = 0;
    pipeline->occupied = 0;
    mutex_unlock(&msdisp_drm->pipeline_lock);
}
EXPORT_SYMBOL(msdisp_drm_release_pipeline);

int msdisp_drm_get_pipeline_global_id(struct drm_device* drm, int pipeline_index)
{
    int drm_index;
    if (!drm ) {
        return -1;
    }

    if (pipeline_index >= MSDISP_DRM_MAX_PIPELINE_CNT) {
        return -1;
    }

    drm_index = msdisp_drm_get_drm_device_index(drm);
    if (drm_index < 0) {
        return -1;
    }

    return drm_index * MSDISP_DRM_MAX_PIPELINE_CNT + pipeline_index;
}
EXPORT_SYMBOL(msdisp_drm_get_pipeline_global_id);

struct kobject* msdisp_drm_get_pipeline_kobject(struct drm_device* drm, int pipeline_index)
{
    struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);

    if (!drm ) {
        return NULL;
    }

    if (pipeline_index >= MSDISP_DRM_MAX_PIPELINE_CNT) {
        return NULL;
    }

    return &msdisp_drm->pipeline[pipeline_index].dev.kobj;
}
EXPORT_SYMBOL(msdisp_drm_get_pipeline_kobject);

int msdisp_drm_register_usb_hal(struct drm_device* drm, int pipeline_index, struct msdisp_usb_hal* usb_hal)
{
    struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);
    struct msdisp_drm_pipeline* pipeline;
    int ret;

    if (!drm || !usb_hal) {
        return -EINVAL;
    }

    if (pipeline_index < 0 || pipeline_index >= msdisp_drm->pipeline_cnt) {
        return -EINVAL;
    }

    pipeline = &msdisp_drm->pipeline[pipeline_index];
    mutex_lock(&msdisp_drm->pipeline_lock);
    if (pipeline->reg_flag) {
        mutex_unlock(&msdisp_drm->pipeline_lock);
        return -EBUSY;
    }
    pipeline->occupied = 1;
    pipeline->reg_flag = 1;
    mutex_unlock(&msdisp_drm->pipeline_lock);

    mutex_lock(&pipeline->hal_lock);
    pipeline->usb_hal = usb_hal;
    if (pipeline->connector) {
        pipeline->connector->force_edid_refresh = true;
    }
    if (MSDISP_DRM_STATUS_ENABLE == pipeline->drm_status) {
        ret = usb_hal->funcs->enable(usb_hal, pipeline->drm_width, pipeline->drm_height, pipeline->drm_rate, pipeline->drm_fb_format);
    }
    mutex_unlock(&pipeline->hal_lock);
    (void)ret;
    return 0;
}
EXPORT_SYMBOL(msdisp_drm_register_usb_hal);

int msdisp_drm_unregister_usb_hal(struct drm_device* drm, int pipeline_index)
{
    struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);
    struct msdisp_drm_pipeline* pipeline;
    if (!drm) {
        return -EINVAL;
    }

    if (pipeline_index < 0 || pipeline_index >= msdisp_drm->pipeline_cnt) {
        return -EINVAL;
    }

    pipeline = &msdisp_drm->pipeline[pipeline_index];
    mutex_lock(&pipeline->hal_lock);
    pipeline->usb_hal = NULL;
    if (pipeline->connector) {
        pipeline->connector->force_edid_refresh = true;
    }
    mutex_unlock(&pipeline->hal_lock);

    mutex_lock(&msdisp_drm->pipeline_lock);
    pipeline->reg_flag = 0;
    mutex_unlock(&msdisp_drm->pipeline_lock);
    return 0;
}
EXPORT_SYMBOL(msdisp_drm_unregister_usb_hal);

struct kfifo* msdisp_drm_get_kfifo(struct drm_device* drm, int pipeline_index)
{
    struct msdisp_drm_device* msdisp_drm = to_msdisp_drm(drm);

    if (!drm) {
        return NULL;
    }

    if (pipeline_index >= MSDISP_DRM_MAX_PIPELINE_CNT) {
        return NULL;
    }

    return &msdisp_drm->pipeline[pipeline_index].fifo;
}
EXPORT_SYMBOL(msdisp_drm_get_kfifo);


