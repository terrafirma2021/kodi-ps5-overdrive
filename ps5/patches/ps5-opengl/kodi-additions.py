#!/usr/bin/env python3
"""Expose the native VideoOut handle used by Kodi's display settings."""

from pathlib import Path
import sys


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: kodi-additions.py <ps5-opengl source>")
    source = Path(sys.argv[1]) / "src/platform/ps5_agc_native_runtime.c"
    content = source.read_text()
    if "int ps5_opengl_video_out_handle(void)" in content:
        return
    anchor = "static int runtime_video_handle = -1;"
    if content.count(anchor) != 1:
        raise SystemExit(f"unexpected VideoOut runtime layout: {source}")
    addition = """

/* Kodi queries the native display handle for resolution and HDR settings. */
int ps5_opengl_video_out_handle(void)
{
    return runtime_video_handle;
}
"""
    source.write_text(content.replace(anchor, anchor + addition, 1))


if __name__ == "__main__":
    main()
