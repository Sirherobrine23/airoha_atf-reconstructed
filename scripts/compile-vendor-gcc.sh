#!/bin/sh
set -eu

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "usage: $0 SOURCE.c OUTPUT.o [CC]" >&2
    exit 2
fi

src=$1
out=$2
cc=${3:-${VENDOR_CC:-}}

if [ -z "$cc" ]; then
    for p in \
      /opt/trendchip/buildroot-gcc1030-glibc232_kernel5_4/usr/bin/arm-linux-gcc \
      /opt/trendchip/buildroot-gcc1030-glibc232_kernel5_4/bin/arm-linux-gcc; do
        if [ -x "$p" ]; then cc=$p; break; fi
    done
fi

if [ -z "$cc" ] || [ ! -x "$cc" ]; then
    echo "vendor GCC not found; set VENDOR_CC=/path/to/arm-linux-gcc" >&2
    exit 1
fi

inc=$(dirname "$src")
exec "$cc" \
    -mthumb -mno-unaligned-access -march=armv8-a+crc \
    -ffunction-sections -fdata-sections -ffreestanding \
    -fno-builtin -fno-common -Os -std=gnu99 \
    -I"$inc" -c "$src" -o "$out"
