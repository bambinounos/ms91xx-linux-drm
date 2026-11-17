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
 * usb_hal_sysfs.c -- Drm driver for MacroSilicon chip 913x and 912x
 */

#include <linux/types.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/usb.h>

#include "hal_adaptor.h"
#include "usb_hal_dev.h"
#include "usb_hal_interface.h"

static ssize_t usb_hal_buf_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
    struct usb_hal_dev* usb_dev;
	struct usb_hal_buffer* usb_buf;
	char tmp[256];

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;
	usb_buf = &usb_dev->usb_buf;

	*buf = 0;
	sprintf(tmp, "%s\n", dev->kobj.name);
	strcat(buf, tmp);
	sprintf(tmp, "buf size:%d\n", usb_buf->size);
	strcat(buf, tmp);
	sprintf(tmp, "buf len:%d\n", usb_buf->len);
	strcat(buf, tmp);
	sprintf(tmp, "buf type:%d\n", usb_buf->type);
	strcat(buf, tmp);

	return strlen(buf);
}

static ssize_t usb_hal_frame_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
    struct usb_hal_dev* usb_dev;
    struct usb_hal_dev_frame_stat* stat;
	char tmp[256];

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;
	stat = &usb_dev->stat;

	*buf = 0;

	sprintf(tmp, "send total:%lld\n", stat->send_total);
	strcat(buf, tmp);
	sprintf(tmp, "send success:%lld\n", stat->send_success);
	strcat(buf, tmp);
	sprintf(tmp, "update event count:%lld\n", stat->update_event);
	strcat(buf, tmp);
	// sprintf(tmp, "period send count:%lld\n", stat->period_send);
	// strcat(buf, tmp);
	sprintf(tmp, "state error count:%lld\n", stat->state_error);
	strcat(buf, tmp);
	sprintf(tmp, "try lock fail:%lld\n", stat->try_lock_fail);
	strcat(buf, tmp);
	sprintf(tmp, "empty rect skip:%lld\n", stat->empty_rect_skip);
	strcat(buf, tmp);
	sprintf(tmp, "short send:%lld\n", stat->short_send);
	strcat(buf, tmp);
	
	return strlen(buf);
}

static ssize_t usb_hal_dev_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
	struct usb_hal_dev* usb_dev;
	char tmp[96];

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;

	*buf = 0;

	sprintf(tmp, "chip id:0x%x\n", usb_hal->chip_id);
	strcat(buf, tmp);
	sprintf(tmp, "video port:0x%x\n", usb_hal->port_type);
	strcat(buf, tmp);
	sprintf(tmp, "sdram type:0x%x\n", usb_hal->sdram_type);
	strcat(buf, tmp);
	sprintf(tmp, "dev state:0x%x\n", usb_dev->state);
	strcat(buf, tmp);
	
	return strlen(buf);
}

static ssize_t usb_hal_custom_mode_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
	struct usb_hal_dev* usb_dev;
	ssize_t len = 0;
	int i;

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;

	len += scnprintf(buf + len, PAGE_SIZE - len, "custom mode cnt:%d\n", usb_dev->custom_mode_cnt);
	for (i = 0; i < usb_dev->custom_mode_cnt; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len, "width:%d height:%d rate:%d vid:%d\n",
				 usb_dev->custom_mode[i].width, usb_dev->custom_mode[i].height,
				 usb_dev->custom_mode[i].rate, usb_dev->custom_mode[i].vic);
	}

	return len;
}

static ssize_t usb_hal_custom_mode_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
	char *ori, *start, *cur;
	int added = 0;

	if (!usb_hal)
		return -ENODEV;

	ori = kstrndup(buf, count, GFP_KERNEL);
	if (!ori)
		return -ENOMEM;

	strim(ori);
	if (strcmp(ori, "clear") == 0 || strcmp(ori, "none") == 0) {
		usb_hal_clear_custom_modes(usb_hal);
		kfree(ori);
		msdisp_usb_notify_hotplug(dev);
		return count;
	}

	usb_hal_clear_custom_modes(usb_hal);
	start = ori;
	while ((cur = strsep(&start, ",")) != NULL) {
		int width = 0, height = 0, rate = 0, vic = 0;
		char trailing = 0;

		cur = strim(cur);
		if (!*cur)
			continue;

		if (sscanf(cur, "%d_%dx%d@%d%c", &vic, &width, &height, &rate, &trailing) != 4 ||
		    width <= 0 || width > 0xffff || height <= 0 || height > 0xffff ||
		    rate <= 0 || rate > 0xff || vic <= 0 || vic > 0xff) {
			kfree(ori);
			return -EINVAL;
		}
		if (usb_hal_add_custom_mode(usb_hal, width, height, rate, (unsigned char)vic) == 0)
			added++;
	}

	kfree(ori);
	if (added == 0)
		return -EINVAL;

	msdisp_usb_notify_hotplug(dev);
	return count;
}

static ssize_t usb_hal_port_type_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);

	if (!usb_hal)
		return -ENODEV;

	return scnprintf(buf, PAGE_SIZE, "%u\n", usb_hal->port_type);
}

static ssize_t usb_hal_port_type_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
	int port_val;
	int ret;

	if (!usb_hal)
		return -ENODEV;

	ret = kstrtoint(buf, 0, &port_val);
	if (ret < 0)
		return ret;

	ret = usb_hal_set_port_type(usb_hal, port_val);
	if (ret < 0)
		return ret;

	msdisp_usb_notify_hotplug(dev);
	return count;
}

static ssize_t usb_hal_write_xdata_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
    struct usb_hal_dev* usb_dev;
	const struct msdisp_hal_dev* hal_dev;
	u32 reg, data;
	u8 read_data;
	int ret;

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;
	hal_dev = usb_dev->hal_dev;

	ret = sscanf(buf, "%x %x\n", &reg, &data);
	if (ret != 2)
		return -EINVAL;

	ret = hal_dev->funcs->xdata_write_byte(usb_dev->udev, (u16)reg, (u8)data);
	if (ret < 0)
		return ret;

	ret = hal_dev->funcs->xdata_read_byte(usb_dev->udev, reg, &read_data);
	if (ret < 0)
		return ret;
	
	dev_info(dev, "the reg:0x%x data:0x%02x read_data:0x%02x\n", reg, data, read_data);
	return count;
}

static ssize_t usb_hal_read_xdata_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    struct usb_hal* usb_hal = usb_intf_device_to_hal_func(dev);
    struct usb_hal_dev* usb_dev;
	const struct msdisp_hal_dev* hal_dev;
	u16 reg;
	int ret;
	u8 data;

	if (!usb_hal)
		return -ENODEV;
	usb_dev = usb_hal->private;
	hal_dev = usb_dev->hal_dev;

	ret = kstrtou16(buf, 0, &reg);
	if (ret < 0)
		return ret;

	ret = hal_dev->funcs->xdata_read_byte(usb_dev->udev, reg, &data);
	if (ret < 0)
		return ret;
	
	dev_info(dev, "the reg:0x%x data:0x%02x\n", reg, data);
	return count;
}


static DEVICE_ATTR(buf, 0444, usb_hal_buf_show, NULL);
static DEVICE_ATTR(frame, 0444, usb_hal_frame_show, NULL);
static DEVICE_ATTR(hal_dev, 0444, usb_hal_dev_show, NULL);
static DEVICE_ATTR(custom_mode, 0644, usb_hal_custom_mode_show, usb_hal_custom_mode_store);
static DEVICE_ATTR(port_type, 0644, usb_hal_port_type_show, usb_hal_port_type_store);
static DEVICE_ATTR(write_xdata, 0220, NULL, usb_hal_write_xdata_store);
static DEVICE_ATTR(read_xdata, 0220, NULL, usb_hal_read_xdata_store);


static struct attribute* usb_hal_attribute[] = {
	&dev_attr_buf.attr,
	&dev_attr_frame.attr,
	&dev_attr_hal_dev.attr,
	&dev_attr_custom_mode.attr,
	&dev_attr_port_type.attr,
	&dev_attr_write_xdata.attr,
	&dev_attr_read_xdata.attr,
	NULL
};

static const struct attribute_group usb_hal_attr_group = {
	.attrs = usb_hal_attribute,
};

void usb_hal_sysfs_init(struct usb_interface *interface)
{
    int r;
	r = sysfs_create_group(&interface->dev.kobj, &usb_hal_attr_group);
	if (r) {
		dev_err(&interface->dev, "create sysfs group fialed! ret=%d\n", r);
	}
}

void usb_hal_sysfs_exit(struct usb_interface *interface)
{
	sysfs_remove_group(&interface->dev.kobj, &usb_hal_attr_group);
}