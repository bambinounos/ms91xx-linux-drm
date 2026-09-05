# MacroSilicon MS91xx Linux DRM driver (modern-kernel fork)

Linux DRM driver for **MacroSilicon MS9132 / MS9133 / MS9135** USB display chips
(`lsusb`: **`345f:9132` / `345f:9133` "MS USB Video"**), based on MacroSilicon's
**official GPL-2.0 source code**, fixed to build and run on **modern kernels
(6.8 → 7.0+)** and packaged for **DKMS**.

These chips power countless **brandless "lightweight" portable USB monitors** and
**Chinese dual-screen / tri-screen laptop extenders** sold on AliExpress, Temu,
Amazon, etc., as well as generic **USB 3.0 → HDMI/VGA adapters**.

> ✅ Tested working on **Ubuntu 24.04 (Noble), kernel 6.17.0-35-generic, GNOME 46
> Wayland** with a brandless dual-screen laptop monitor (MS9132 in single mode,
> 1920×1080@60, USB 3 SuperSpeed) — both **mirror** and **extend** modes.

## Is this my device?

```
$ lsusb | grep 345f
Bus 004 Device 003: ID 345f:9132 MS           USB Video
```

If you see `345f:9132`, `345f:9133` or `345f:9135`, yes.

If you see **`534d:6021`** ("MacroSilicon USB Video" / "VGA Display Adapter"),
also yes — that is the functional identity these dongles mode-switch to on their
own after enumerating as `345f:9132`. USB→**VGA** adapters in that family are
usually **MS912C** chips (`dmesg` reports `chip id:0x3`) and need two extra
module options to produce any picture at all; see
**[docs/MS912C-VGA.md](docs/MS912C-VGA.md)**.

Typical symptoms before installing this driver:

- The monitor only shows a **red/standby LED** on Linux (works fine on Windows).
- The device **re-enumerates every ~10 seconds** (`dmesg` shows connect/disconnect
  loops) because no driver claims its video interface.
- With the reverse-engineered `ms912x` driver, the panel **flickers and then shows
  "No Signal"** (see [rhgndf/ms912x#42](https://github.com/rhgndf/ms912x/issues/42)),
  or shows corrupted/magenta images. This driver fixes that: the official enable
  sequence keeps the video output muted until the first frame has been transferred,
  which the dual-screen (MS9133-family) firmware requires.

## Install (DKMS, recommended)

```sh
git clone https://github.com/bambinounos/ms91xx-linux-drm.git
cd ms91xx-linux-drm
sudo rsync -a --exclude .git ./ /usr/src/msdisp-3.0.3.13/
sudo dkms add msdisp/3.0.3.13
sudo dkms install msdisp/3.0.3.13
```

Plug the screen in (or unplug/replug it). The `usbdisp_usb` module auto-loads via
the USB modalias, a new DRM card appears, and your desktop (tested on GNOME
Wayland/Mutter) lights the panel up like any other monitor. DKMS rebuilds the
modules automatically on every kernel update.

### Manual build (without DKMS)

```sh
make                          # builds against the running kernel
sudo insmod drm/usbdisp_drm.ko
sudo insmod drm/usbdisp_usb.ko
```

## What this fork fixes (vs. official V3.0.3.13)

MacroSilicon's last public release targets kernels up to ~6.6. On newer kernels it
does not compile — and even when it compiles, on **6.12+ the DRM node cannot be
opened at all** (every `open()` of `/dev/dri/cardN` fails with `EINVAL`), so
compositors silently ignore the device. All fixes are guarded with
`LINUX_VERSION_CODE`, preserving compatibility with old kernels:

| Fix | Affected kernels |
|---|---|
| `.fop_flags = FOP_UNSIGNED_OFFSET` in fops — **without this, `drm_open_helper()` rejects every open and the GPU is unusable** | ≥ 6.12 |
| `platform_driver.remove` returns `void` | ≥ 6.11 |
| `drm_helper_mode_fill_fb_struct()` / `fb_create` take `const struct drm_format_info *` | ≥ 6.15 |
| `drm_do_get_edid()` removed → ported to `drm_edid_read_custom()` (raw EDID copy kept, rest of the flow unchanged) | ≥ 6.13 |
| `connector->mode_valid()` takes `const struct drm_display_mode *` | ≥ 6.15 |
| `MODULE_IMPORT_NS("...")` requires a string literal | ≥ 6.13 |
| `from_timer()` → `timer_container_of()`, `del_timer()` → `timer_delete()` | ≥ 6.16 / 6.2 |
| `.date` removed from `struct drm_driver`; PRIME fd/handle hooks now core defaults | ≥ 6.14 / 6.6 |
| `drm_legacy.h` deleted; missing `vmalloc.h` / `scatterlist.h` includes | ≥ 6.8 |
| `struct_mutex` removed from `drm_device` (GEM locking is internal); explicit `drm_print.h` includes; `DRM_DEBUG_PRIME` → `drm_dbg_prime()` | ≥ 7.0 |
| Makefiles: removed vestigial `KERNELRELEASE`/`include Kbuild` branch that **broke DKMS builds**; `$(PWD)` → `$(CURDIR)`; added `dkms.conf` | all |
| `drm_do_get_edid()` guard lowered to ≥ 6.12 — Debian's `6.12.101+deb13` backports the 6.13 EDID API while still reporting 6.12, so the build failed | ≥ 6.12 (Debian) |
| `.fbdev_probe` implemented + GEM `.vmap`/`.vunmap` + `drm_client_setup()` (adapted for `drm_client_buffer_create_dumb()` and pre-allocated `fb_info` on 7.0+) — provides `/dev/fb0` and text console support | ≥ 6.10 |
| `534d:6021` added to all three ID lookup tables; `port_type=` override; `custom_mode=` modes honoured with EDID fallback for standard HDMI — see [docs/MS912C-VGA.md](docs/MS912C-VGA.md) | all |

The protocol/init logic is **untouched** — it is exactly the manufacturer's code.

## Known issues (inherited from the vendor code)

- **Do not `rmmod usbdisp_drm`.** The vendor teardown path has a use-after-free
  that oopses the kernel and leaves the module stuck (refcount -1). To reload the
  driver, reboot. In normal use the modules simply stay loaded.
- If the panel shows only the red LED and **never enumerates** (no `dmesg` events
  at all when plugging), the chip's firmware is latched: disconnect the USB cable
  *and any other power source* for ~2 minutes, then reconnect.
- A ~10 s connect/disconnect loop in `dmesg` is normal **only while no driver is
  bound**; it stops as soon as `usbdisp_usb` claims the device.

## Credits & provenance

- **MacroSilicon Technology Co., Ltd.** — original driver, licensed GPL-2.0.
  Source obtained from their public download server:
  `http://www.macrosilicon.com:9080/download/USBDisplay/Linux/SourceCode/`
  (`MS91xx_Linux_Drm_SourceCode_V3.0.3.13.zip`, 2025-07-10 — the pristine zip is
  kept in this repo for provenance).
- **[rhgndf/ms912x](https://github.com/rhgndf/ms912x)** and contributors — the
  reverse-engineering work that kept these devices alive on Linux for years and
  whose issue tracker pointed the way here.
- Modern-kernel fixes and DKMS packaging: this fork (debugged live against a real
  dual-screen MS9132/MS9133 device, including USB protocol captures of the
  Windows driver).

## License

GPL-2.0 (same as the original MacroSilicon code). See [LICENSE](LICENSE).

---

## Guía rápida en español

Driver Linux para los chips **MacroSilicon MS9132/MS9133** (`lsusb`: `345f:9132`)
que usan las **pantallas portátiles chinas sin marca** y los **monitores duales
para laptop** ("dual screen", "lightweight monitor"). Es el código **oficial GPL
del fabricante**, corregido para kernels modernos (probado en Ubuntu 24.04 con
kernel 6.17, GNOME Wayland, modos espejo y extendido).

```sh
git clone https://github.com/bambinounos/ms91xx-linux-drm.git
cd ms91xx-linux-drm
sudo rsync -a --exclude .git ./ /usr/src/msdisp-3.0.3.13/
sudo dkms add msdisp/3.0.3.13
sudo dkms install msdisp/3.0.3.13
```

Conecta la pantalla y listo: aparece como un monitor más. Consejos: nunca hagas
`rmmod usbdisp_drm` (bug del fabricante: para recargar, reinicia); si la pantalla
queda solo con el LED rojo y no aparece en `lsusb`, desconéctala de todo ~2
minutos y vuelve a conectarla.
