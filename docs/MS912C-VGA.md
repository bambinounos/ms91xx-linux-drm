# MS912C USB→VGA adapters (`534d:6021`)

This documents the changes needed to make a **VGA** dongle built around an
**MS912C** work with this driver, on a machine with no other graphics card,
as the **physical text console** (fbcon on `/dev/fb0`).

Tested on Debian 13, kernel `6.12.101+deb13-amd64`, with a 15-year-old
DDC-less VGA monitor. Console text renders and updates correctly at
640×480@60. Nothing here was tested under a desktop compositor.

## Is this my device?

```
$ lsusb
Bus 003 Device 006: ID 534d:6021 MacroSilicon USB Video
$ dmesg | grep 'chip id'
usb 3-9: chip id:0x3 port:0x1 sdram:0x2
```

Two things distinguish it from the MS9132/9133/9135 devices this driver was
written for:

- **It changes its USB ID.** `345f:9132` is only the fake driver-CD identity
  it enumerates with at first; it mode-switches on its own — no
  `usb_modeswitch` involved — to `534d:6021`, a composite HID + Audio +
  vendor-display device, and stays there. All three of the driver's ID
  lookup tables only knew about the `345f:*` IDs, so nothing bound.
- **`chip id:0x3` is `CHIP_ID_912C`**, not `CHIP_ID_9132` (`0x0`). See
  `usb_hal/usb_hal_chip.h`. This matters: `ms9132_set_screen_enable()`
  branches on chip id, and only the 9132 takes the simple path.

## Setup

Install as usual (see the main README), then create
`/etc/modprobe.d/usbdisp.conf`:

```
options usbdisp_usb port_type=2 custom_mode=1_640x480@60,4_1280x720@60
```

Both options are required on this hardware, for unrelated reasons.

### `port_type=2` — the VGA DAC is otherwise never powered on

The chip's video-port register reports `1` (`VIDEO_PORT_SVIDEO`) even though
the physical connector is genuinely VGA. `ms9132_set_screen_enable()`'s
`switch (port_type)` has no `VIDEO_PORT_SVIDEO` case, so it falls through to
`default:` — xdata `0xf004` bit `0x2`, the CVBS/S-Video output enable —
instead of the `VIDEO_PORT_VGA` case, xdata `0xf004` bit `0x80`, the **VGA
DAC enable**.

The symptom is maximally misleading: frames stream over USB, the chip ACKs
everything, `set_video_enable()` and `set_screen_enable()` both return 0,
`start video success!` is logged, `/dev/fb0` has live pixel data — and the
monitor reports no signal, because no analog signal is ever generated.

`port_type=<n>` overrides the detected value (`-1`, the default, keeps
autodetection). Values are the `VIDEO_PORT_*` enum in
`usb_hal/usb_hal_chip.h`: `0` CVBS, `1` SVIDEO, **`2` VGA**, `3` YPBPR,
`4` CVBS_SVIDEO, `5` HDMI, `6` DIGITAL.

Note that the override does **two** things, and only doing the first is not
enough: besides fixing the driver's own copy, it writes the value back into
the chip at `MS9132_XDATA_REG_VIDEO_PORT` (xdata `0x31`). The chip's firmware
reads that same register when it programs its video timing generator during
the enable sequence, so left at `1` it emits S-Video-style timing that a VGA
monitor cannot lock onto — the DAC is on, and the monitor still sleeps. The
register is plain writable RAM, but volatile: it resets with the dongle's
power, so the driver re-applies it on every init.

Confirm it took effect:

```
$ dmesg | grep 'video port'
usb 3-9: video port type overridden: 1 -> 2
```

### `custom_mode=` — no EDID to read

A basic VGA monitor and cable have no DDC, so there is no EDID and the driver
has no mode list. `custom_mode` supplies one directly, as
`<vic>_<w>x<h>@<rate>` pairs. VIC 1 (640×480@60) is the universal fallback;
VIC 4 (1280×720@60) is used if the display syncs to it. **Only CEA-861 VICs
work** — the timings come from a fixed table in `drm/msdisp_drm_mode.c`, so
arbitrary VESA resolutions such as 1024×768 are not available through this
path. The first entry listed is marked preferred.

Do not omit this. The EDID read on this hardware does not fail cleanly — it
intermittently "succeeds" from one boot to the next with what looks like a
chip-internal canned EDID. Its established timings (e.g. 800×600@60) then win
as the preferred mode, the chip maps that unconfigured resolution to an
unrelated internal VIC, and the output is invalid: another black screen, no
error anywhere. The connector code therefore ignores EDID modes entirely and
uses only this list.

## Debugging with the chip's xdata registers

The driver exposes the chip's xdata space over sysfs, on the display control
interface (`3-9:1.3` below — substitute your own bus path):

```sh
cd /sys/devices/pci0000:00/0000:00:14.0/usb3/3-9/3-9:1.3
echo 0xf004 > read_xdata      # result appears in dmesg
echo "31 2"  > write_xdata    # write 0x02 to 0x31, reads back
```

Both attributes are write-only and report through `dmesg`. This is by far the
fastest way to test a theory about the chip — no rebuild, no reboot.

To make the chip re-run its enable sequence after poking a register, blank and
unblank the framebuffer:

```sh
echo 4 > /sys/class/graphics/fb0/blank
echo 0 > /sys/class/graphics/fb0/blank
```

`dmesg` shows `disable hw begin/end` and then `start video success!`. This is
also the safe way to re-initialise the video pipeline: never `rmmod
usbdisp_drm` (see "Known issues" in the main README — the vendor teardown path
oopses the kernel).

## Timeline of failure modes

Worth recording, because each of these looks like a hardware fault and none of
them is:

1. Device enumerates, no driver binds — missing `534d:6021` in the ID tables.
2. `Can't find usb hal funcs!` — the same ID missing from the second of three
   lookup tables.
3. Connector reports connected, no `/dev/fb0` ever appears, `NULL pointer
   dereference` at `drm_fb_helper_single_fb_probe()` — no `.fbdev_probe`
   implementation.
4. `/dev/fb0` live, fbcon drawing, USB frames confirmed on the wire with
   `usbmon`, `start video success!` — VGA DAC never enabled (`port_type`).
5. DAC enabled, still no image — the chip's own firmware still reading
   S-Video from xdata `0x31`.

The general lesson from (4) and (5): when a driver reads a hardware
configuration value from a chip register and that value is wrong, overriding
the driver's in-memory copy may not be enough. On-chip firmware often reads
the same register for its own configuration — check whether it is writable and
fix it at the source.
