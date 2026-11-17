#!/usr/bin/env python3
"""Regression test for VID:PID-scoped and global multi-adapter module parameters.

Run with: python3 tests/test_multi_device_params.py
Requires a C compiler (CC, or cc). No kernel headers or hardware are needed.
Compiles the actual parameter-parsing functions from usb_hal_interface.c and
msdisp_usb_drv.c.
"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HAL_SRC = ROOT / 'usb_hal' / 'usb_hal_interface.c'
USB_DRV_SRC = ROOT / 'drm' / 'msdisp_usb_drv.c'


def extract_func(text: str, start_marker: str, end_marker: str) -> str:
    start = text.index(start_marker)
    end = text.index(end_marker, start)
    return text[start:end]


def main():
    hal_text = HAL_SRC.read_text()
    usb_text = USB_DRV_SRC.read_text()

    port_fn = extract_func(
        hal_text,
        'static int usb_hal_resolve_port_type_override(',
        '\nint usb_hal_set_port_type(',
    )
    struct_def = extract_func(
        usb_text,
        'struct custom_mode_stru {',
        '\nstatic DEFINE_MUTEX(',
    )
    mode_fn = extract_func(
        usb_text,
        'static int parse_custom_mode_token(',
        '\nvoid parser_custom_mode(',
    )

    harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint16_t u16;
#define EINVAL 22

PORT_FN
STRUCT_DEF
MODE_FN

int main(void)
{
    /* 1. Unset / empty port_type keeps autodetection (-1). */
    assert(usb_hal_resolve_port_type_override(NULL, 0x345f, 0x9132) == -1);
    assert(usb_hal_resolve_port_type_override("", 0x345f, 0x9132) == -1);

    /* 2. Legacy global port_type=2 applies to any adapter. */
    assert(usb_hal_resolve_port_type_override("2", 0x534d, 0x6021) == 2);
    assert(usb_hal_resolve_port_type_override("2", 0x345f, 0x9132) == 2);

    /* 3. Scoped port_type=534d:6021:2 overrides ONLY 534d:6021, leaving 345f:9132 on autodetect (-1). */
    assert(usb_hal_resolve_port_type_override("534d:6021:2", 0x534d, 0x6021) == 2);
    assert(usb_hal_resolve_port_type_override("534d:6021:2", 0x345f, 0x9132) == -1);

    /* 4. Combined global fallback + per-VID:PID exception is order-independent. */
    assert(usb_hal_resolve_port_type_override("2,345f:9132:-1", 0x345f, 0x9132) == -1);
    assert(usb_hal_resolve_port_type_override("2,345f:9132:-1", 0x534d, 0x6021) == 2);
    assert(usb_hal_resolve_port_type_override("345f:9132:-1,2", 0x345f, 0x9132) == -1);
    assert(usb_hal_resolve_port_type_override("345f:9132:5, 534d:6021:2", 0x345f, 0x9132) == 5);
    assert(usb_hal_resolve_port_type_override("345f:9132:5, 534d:6021:2", 0x534d, 0x6021) == 2);

    /* 5. Legacy unscoped custom_mode token parsing. */
    struct custom_mode_stru m;
    assert(parse_custom_mode_token("1_640x480@60", &m) == 0);
    assert(!m.has_vid_pid);
    assert(m.vic == 1 && m.width == 640 && m.height == 480 && m.rate == 60);

    /* 6. VID:PID-scoped custom_mode token parsing (mixed MS9132 HDMI + MS912C VGA). */
    assert(parse_custom_mode_token("534d:6021:4_1280x720@60", &m) == 0);
    assert(m.has_vid_pid);
    assert(m.match_vid == 0x534d && m.match_pid == 0x6021);
    assert(m.vic == 4 && m.width == 1280 && m.height == 720 && m.rate == 60);

    /* 7. Reject malformed tokens cleanly. */
    assert(parse_custom_mode_token("", &m) != 0);
    assert(parse_custom_mode_token("1_640x480@60junk", &m) != 0);
    assert(parse_custom_mode_token("1ffff:6021:1_640x480@60", &m) != 0);
    assert(parse_custom_mode_token("0_640x480@60", &m) != 0);

    printf("PASS: multi-adapter port_type and custom_mode parsing verified\n");
    return 0;
}
'''
    harness = (
        harness
        .replace('PORT_FN', port_fn)
        .replace('STRUCT_DEF', struct_def)
        .replace('MODE_FN', mode_fn)
    )

    with tempfile.TemporaryDirectory(prefix='msdisp-param-test-') as directory:
        c_file = Path(directory) / 'test.c'
        executable = Path(directory) / 'test'
        c_file.write_text(harness)
        subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + [
            '-std=c99', '-Wall', '-Wextra', '-Werror',
            str(c_file), '-o', str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    main()
