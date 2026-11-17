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
 * msdisp_usb_drv.c -- Drm driver for MacroSilicon chip 913x and 912x
 */
 
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <linux/mm_types.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/completion.h>
#include <linux/scatterlist.h>
#include <linux/kfifo.h>

#include <drm/drm_device.h>
#include <drm/drm_modeset_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_file.h>
#include <drm/drm_fourcc.h>

#include "msdisp_usb_interface.h"
#include "msdisp_drm_interface.h"
#include "msdisp_usb_drv.h"
#include "usb_hal_interface.h"

#if KERNEL_VERSION(6, 13, 0) <= LINUX_VERSION_CODE
MODULE_IMPORT_NS("usbdisp_drm");
#elif KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE
MODULE_IMPORT_NS(usbdisp_drm);
#endif

#define MOD_VER							"3.1.7"
#define MAX_CUSTOM_MODE_CNT				16

struct custom_mode_stru {
	unsigned short match_vid;
	unsigned short match_pid;
	bool has_vid_pid;
	unsigned short width;
	unsigned short height;
	unsigned char rate;
	unsigned char vic;
};


static DEFINE_MUTEX(option_parse_lock);
static int option_parserd = 0;
static char* custom_mode = NULL;
static int custom_mode_cnt = 0;
static struct custom_mode_stru mode_arr[MAX_CUSTOM_MODE_CNT];

module_param(custom_mode, charp, S_IRUGO);
MODULE_PARM_DESC(custom_mode,
	"custom mode: ([<vid>:<pid>:]<vic>_<x>x<y>@<refr>[,...], e.g. 1_640x480@60 or 534d:6021:1_640x480@60)");


void msdisp_usb_sysfs_init(struct usb_interface *interface);
void msdisp_usb_sysfs_exit(struct usb_interface *interface);


static int parse_custom_mode_token(const char *raw, struct custom_mode_stru *out)
{
	char tok[64];
	size_t len;
	unsigned int m_vid = 0, m_pid = 0;
	int width = 0, height = 0, rate = 0, vic = 0;
	char trailing = 0;

	if (!raw || !out)
		return -EINVAL;

	while (*raw == ' ' || *raw == '\t')
		raw++;
	len = strlen(raw);
	while (len > 0 && (raw[len - 1] == ' ' || raw[len - 1] == '\t' ||
			   raw[len - 1] == '\n' || raw[len - 1] == '\r'))
		len--;
	if (len == 0 || len >= sizeof(tok))
		return -EINVAL;

	memcpy(tok, raw, len);
	tok[len] = '\0';

	memset(out, 0, sizeof(*out));
	if (strchr(tok, ':')) {
		if (sscanf(tok, "%x:%x:%d_%dx%d@%d%c",
			   &m_vid, &m_pid, &vic, &width, &height, &rate, &trailing) != 6)
			return -EINVAL;
		if (m_vid > 0xffff || m_pid > 0xffff)
			return -EINVAL;
		out->has_vid_pid = true;
		out->match_vid = (unsigned short)m_vid;
		out->match_pid = (unsigned short)m_pid;
	} else {
		if (sscanf(tok, "%d_%dx%d@%d%c",
			   &vic, &width, &height, &rate, &trailing) != 4)
			return -EINVAL;
		out->has_vid_pid = false;
	}

	if (width <= 0 || width > 0xffff || height <= 0 || height > 0xffff ||
	    rate <= 0 || rate > 0xff || vic <= 0 || vic > 0xff)
		return -EINVAL;

	out->width = (unsigned short)width;
	out->height = (unsigned short)height;
	out->rate = (unsigned char)rate;
	out->vic = (unsigned char)vic;
	return 0;
}

void parser_custom_mode(void)
{
	char* start, *cur, *ori;
	struct custom_mode_stru parsed;

	custom_mode_cnt = 0;
	memset(&mode_arr, 0, sizeof(mode_arr));

	if (!custom_mode) {
		return ;
	}

	ori = kstrdup(custom_mode, GFP_KERNEL);
	if (!ori) {
		printk("msdisp:duplicate custom mode failed!\n");
		return ;
	}

	start = ori;
	while (start && custom_mode_cnt < MAX_CUSTOM_MODE_CNT) {
		cur = strsep(&start, ",");
		if (!cur) {
			break;
		}
		if (!*cur) {
			continue;
		}

		if (parse_custom_mode_token(cur, &parsed) == 0) {
			if (parsed.has_vid_pid) {
				printk("msdisp:parsed custom mode [%04x:%04x]: width:%d height:%d rate:%d vic:%d\n",
				       parsed.match_vid, parsed.match_pid,
				       parsed.width, parsed.height, parsed.rate, parsed.vic);
			} else {
				printk("msdisp:parsed custom mode: width:%d height %d rate:%d vic:%d\n",
				       parsed.width, parsed.height, parsed.rate, parsed.vic);
			}
			mode_arr[custom_mode_cnt++] = parsed;
		} else {
			printk("msdisp:ignoring invalid custom_mode entry: %s\n", cur);
		}
	}

	kfree(ori);
}

void msdisp_usb_add_custom_mode(struct msdisp_usb_device* usb_dev)
{
	int i;
	int ret;
	struct usb_hal* hal = usb_dev->hal;
	u16 udev_vid = le16_to_cpu(usb_dev->udev->descriptor.idVendor);
	u16 udev_pid = le16_to_cpu(usb_dev->udev->descriptor.idProduct);

	usb_hal_init_gpio(hal);
	usb_hal_read_custom_timing(hal);
	usb_hal_resume_gpio(hal);

	for (i = 0; i < custom_mode_cnt; i++) {
		if (mode_arr[i].has_vid_pid &&
		    (mode_arr[i].match_vid != udev_vid || mode_arr[i].match_pid != udev_pid)) {
			continue;
		}
		ret = usb_hal_add_custom_mode(hal, mode_arr[i].width, mode_arr[i].height, mode_arr[i].rate, mode_arr[i].vic);
		dev_info(&usb_dev->udev->dev,"add custom mode:width:%d height:%d rate:%d vic:%d %s\n", \
			mode_arr[i].width, mode_arr[i].height, mode_arr[i].rate, mode_arr[i].vic, ret ? "failed" : "success");
	}
}

static int msdisp_usb_suspend(struct usb_interface *interface,
			   pm_message_t message)
{
    struct msdisp_usb_device* usb_dev = usb_get_intfdata(interface);
	int ret;

	ret = drm_mode_config_helper_suspend(usb_dev->drm);
	dev_info(&usb_dev->udev->dev, "suspend! ret=%d\n", ret);
    return ret;
}

static int msdisp_usb_resume(struct usb_interface *interface)
{
    struct msdisp_usb_device* usb_dev = usb_get_intfdata(interface);
	int ret;

	ret = drm_mode_config_helper_resume(usb_dev->drm);

	dev_info(&usb_dev->udev->dev, "resume! ret=%d\n", ret);
	return ret;
}

static int msdisp_usb_reset_resume(struct usb_interface *interface)
{
    struct msdisp_usb_device* usb_dev = usb_get_intfdata(interface);
	int ret;
	
	if (usb_dev && usb_dev->hal) {
		usb_hal_resume_gpio(usb_dev->hal);
	}
	ret = drm_mode_config_helper_resume(usb_dev->drm);
	if (usb_dev && usb_dev->drm) {
		drm_kms_helper_hotplug_event(usb_dev->drm);
	}
	dev_info(&usb_dev->udev->dev, "reset resume!ret =%d\n", ret);
	return ret;
}

struct usb_hal* usb_intf_device_to_hal_func(struct device* dev)
{
	struct msdisp_usb_device* usb_dev = dev_get_drvdata(dev);
	return usb_dev ? usb_dev->hal : NULL;
}

void msdisp_usb_notify_hotplug(struct device* dev)
{
	struct msdisp_usb_device* usb_dev = dev_get_drvdata(dev);
	if (usb_dev && usb_dev->drm) {
		drm_kms_helper_hotplug_event(usb_dev->drm);
	}
}

static int msdisp_usb_probe(struct usb_interface *interface,
			const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(interface);
    struct msdisp_usb_device* usb_dev;
    struct msdisp_usb_hal* usb_hal;
	struct kfifo* fifo;
    int ret = 0;
	int global_id;
	bool pipeline_acquired = false;
	struct kobject* obj;

	dev_info(&udev->dev, "module version:%s\n", MOD_VER);
	mutex_lock(&option_parse_lock);
	if (!option_parserd) {
		parser_custom_mode();
		option_parserd = 1;
	}
	mutex_unlock(&option_parse_lock);

    usb_dev = devm_kzalloc(&udev->dev, sizeof(struct msdisp_usb_device), GFP_KERNEL);
    if (!usb_dev) {
        dev_err(&udev->dev, "devm_kzalloc alloc usb_dev failed!\n");
        return -ENOMEM;
    }

    usb_hal = devm_kzalloc(&udev->dev, sizeof(*usb_hal), GFP_KERNEL);
    if (!usb_hal) {
        dev_err(&udev->dev, "devm_kzalloc alloc usb_hal failed!\n");
        return -ENOMEM;
    } 

    usb_dev->udev = udev;
    usb_dev->pipeline_index = -1;
	
    usb_hal->funcs = msdisp_usb_find_usb_hal(id);
    if (!usb_hal->funcs) {
        dev_err(&udev->dev, "Can't find usb hal funcs! vid=0x%x pid=0x%x\n", id->idVendor, id->idProduct);
        return -ENOENT;
    }

	usb_dev->pipeline_index = msdisp_drm_acquire_free_pipeline(&usb_dev->drm);
	if (usb_dev->pipeline_index < 0 || !usb_dev->drm) {
		dev_err(&udev->dev, "acquire free drm pipeline failed!\n");
        ret = -ENODEV;
        goto fail;
	}
	pipeline_acquired = true;

	fifo = msdisp_drm_get_kfifo(usb_dev->drm, usb_dev->pipeline_index);
	if (!fifo) {
		dev_err(&udev->dev, "drm get fifo failed!\n");
        ret = -ENODEV;
        goto fail;
    }

	usb_set_intfdata(interface, usb_dev);
	global_id = msdisp_drm_get_pipeline_global_id(usb_dev->drm, usb_dev->pipeline_index);
	usb_dev->hal = usb_hal_init(interface, id, fifo, global_id);
	if (!usb_dev->hal) {
		dev_err(&udev->dev, "usb hal init failed!!\n");
		ret = -ENODEV;
        goto fail;
	}
	msdisp_usb_add_custom_mode(usb_dev);

    usb_hal->private = usb_dev;
    usb_dev->usb_hal = usb_hal;
    ret = msdisp_drm_register_usb_hal(usb_dev->drm, usb_dev->pipeline_index, usb_hal);
    if (ret) {
        dev_err(&udev->dev, "register usb hal failed!");
        ret = -EINVAL;
        goto fail;
    }

	obj = msdisp_drm_get_pipeline_kobject(usb_dev->drm, usb_dev->pipeline_index);
	if (obj) {
		ret = sysfs_create_link(obj, &interface->dev.kobj, "usb_dev");
		if (ret) {
			dev_err(&udev->dev, "create syslink failed!ret=%d\n", ret);
		}
	}

	drm_kms_helper_hotplug_event(usb_dev->drm);
	
    return 0;

fail:
	if (usb_dev->hal) {
		usb_hal_destroy(usb_dev->hal);
		usb_dev->hal = NULL;
	}
	if (pipeline_acquired) {
		msdisp_drm_release_pipeline(usb_dev->drm, usb_dev->pipeline_index);
	}
	usb_set_intfdata(interface, NULL);

    return ret;
}

static void msdisp_usb_disconnect(struct usb_interface *interface)
{
	struct msdisp_usb_device* usb_dev = usb_get_intfdata(interface);
	struct kobject* obj;

	if (!usb_dev) {
		return;
	}
    
    (void)msdisp_drm_unregister_usb_hal(usb_dev->drm, usb_dev->pipeline_index);
	if (usb_dev->hal) {
		usb_hal_destroy(usb_dev->hal);
		usb_dev->hal = NULL;
	}
	obj = msdisp_drm_get_pipeline_kobject(usb_dev->drm, usb_dev->pipeline_index);
	if (obj) {
		sysfs_remove_link(obj, "usb_dev");
	}
	msdisp_drm_release_pipeline(usb_dev->drm, usb_dev->pipeline_index);
	usb_set_intfdata(interface, NULL);
	if (usb_dev->drm) {
		drm_kms_helper_hotplug_event(usb_dev->drm);
	}
}

static const struct usb_device_id id_table[] = {
	{.idVendor = 0x345f, .idProduct = 0x9132, .bInterfaceClass = 0xff,
	 .bInterfaceSubClass = 0x00,
	 .bInterfaceProtocol = 0x00,
	 .match_flags = USB_DEVICE_ID_MATCH_VENDOR |
            USB_DEVICE_ID_MATCH_PRODUCT |
			USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS |
			USB_DEVICE_ID_MATCH_INT_PROTOCOL,},

	{.idVendor = 0x345f, .idProduct = 0x9133, .bInterfaceClass = 0xff,
	 .bInterfaceSubClass = 0x00,
	 .bInterfaceProtocol = 0x00,
	 .match_flags = USB_DEVICE_ID_MATCH_VENDOR |
            USB_DEVICE_ID_MATCH_PRODUCT |
			USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS |
			USB_DEVICE_ID_MATCH_INT_PROTOCOL,},

	{.idVendor = 0x345f, .idProduct = 0x9135, .bInterfaceClass = 0xff,
	 .bInterfaceSubClass = 0x00,
	 .bInterfaceProtocol = 0x00,
	 .match_flags = USB_DEVICE_ID_MATCH_VENDOR |
            USB_DEVICE_ID_MATCH_PRODUCT |
			USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS |
			USB_DEVICE_ID_MATCH_INT_PROTOCOL,},

	/* Post-modeswitch functional ID: this dongle enumerates as 345f:9132
	 * (fake driver-CD mode) only until something claims it, then it
	 * re-enumerates as 534d:6021 "MacroSilicon VGA Display Adapter" for
	 * its actual HID+Audio+vendor-display composite function. Interface
	 * 3 (bInterfaceClass 0xff/0x00/0x00, iInterface "msusb video") is the
	 * display control interface, same shape as the 345f entries above. */
	{.idVendor = 0x534d, .idProduct = 0x6021, .bInterfaceClass = 0xff,
	 .bInterfaceSubClass = 0x00,
	 .bInterfaceProtocol = 0x00,
	 .match_flags = USB_DEVICE_ID_MATCH_VENDOR |
            USB_DEVICE_ID_MATCH_PRODUCT |
			USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS |
			USB_DEVICE_ID_MATCH_INT_PROTOCOL,},
	{},
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver msdisp_usb_drv = {
	.name = "msdisp_usb",
	.probe = msdisp_usb_probe,
	.disconnect = msdisp_usb_disconnect,
	.suspend = msdisp_usb_suspend,
	.resume = msdisp_usb_resume,
	.reset_resume = msdisp_usb_reset_resume,
	.id_table = id_table,
};
module_usb_driver(msdisp_usb_drv);

MODULE_VERSION(MOD_VER);
MODULE_DESCRIPTION("MacroSilicon USB Display USB Driver");
MODULE_LICENSE("GPL");
