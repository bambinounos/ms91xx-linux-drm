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
 * msdisp_plat_dev.h -- Drm driver for MacroSilicon chip 913x and 912x
 */



#ifndef __MSDISP_PLATFORM_DEV_H__
#define __MSDISP_PLATFORM_DEV_H__

#include <linux/types.h>

struct platform_device_info;
struct platform_device;
struct drm_driver;
struct device;

struct platform_device *msdisp_platform_dev_create(struct platform_device_info *info);
void msdisp_platform_dev_destroy(struct platform_device *dev);

int msdisp_platform_device_probe(struct platform_device *pdev);
int msdisp_platform_device_remove(struct platform_device *pdev);

#endif

