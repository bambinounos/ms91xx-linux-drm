#!/usr/bin/env python3
"""Regression test for partial updates at the right/bottom image boundaries.

Run with: python3 tests/test_damage_bounds.py
Requires a C compiler (CC, or cc). No kernel headers or hardware are needed.
The test compiles the actual rectangle-combination function from the driver.
"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'usb_hal' / 'usb_hal_thread.c'


def main():
    source = SOURCE.read_text()
    start = source.index('static void usb_hal_combine_rects(')
    end = source.index('\nstatic void usb_hal_update_change_rects', start)
    function = source[start:end]
    harness = r'''
#include <assert.h>
#include <stdio.h>
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
struct rect { int left, top, right, bottom; };
struct usb_hal_dev {
    struct { int width, height; } mode;
    struct rect rect[2];
};
FUNCTION
static void check_mode(int width, int height)
{
    struct usb_hal_dev dev = {
        .mode = {width, height},
        .rect = {{width, height, 0, 0}, {width, height, 0, 0}},
    };
    /* A two-pixel update at the last row must fit both frame buffers. */
    usb_hal_combine_rects(&dev, width - 2, height - 1, width, height);
    for (int b = 0; b < 2; b++) {
        struct rect r = dev.rect[b];
        assert(r.left <= width - 2 && r.top <= height - 1);
        assert(r.right == width && r.bottom == height);
        /* Follow the converter's two-pixel reads on each packed RGB row. */
        for (int y = r.top; y < r.bottom; y++) {
            for (int x = r.left; x < r.right; x += 2) {
                assert(x + 1 < width);
                assert(y * width + x + 1 < width * height);
            }
        }
    }
    /* Combining more damage must retain the previous right/bottom edge. */
    usb_hal_combine_rects(&dev, 1, 1, 3, 3);
    for (int b = 0; b < 2; b++) {
        assert(dev.rect[b].left == 0 && dev.rect[b].top == 0);
        assert(dev.rect[b].right == width);
        assert(dev.rect[b].bottom == height);
    }
    printf("PASS: %dx%d boundary updates in both frame buffers\n", width, height);
}
int main(void)
{
    check_mode(1366, 768);
    check_mode(1280, 720);
    check_mode(1920, 1080);
    check_mode(1366, 767); /* Exercise the vertical bound as well. */
    return 0;
}
'''.replace('FUNCTION', function)
    with tempfile.TemporaryDirectory(prefix='msdisp-damage-test-') as directory:
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
