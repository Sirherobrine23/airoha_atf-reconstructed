#!/bin/sh
set -eu
if [ "$#" -ne 3 ]; then
    echo "usage: $0 VENDOR.o CANDIDATE.o FUNCTION" >&2
    exit 2
fi
vendor=$1
candidate=$2
fn=$3
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
objcopy=${OBJCOPY:-llvm-objcopy}
sec=".text.$fn"
"$objcopy" --dump-section "$sec=$tmp/vendor.bin" "$vendor"
"$objcopy" --dump-section "$sec=$tmp/candidate.bin" "$candidate"
printf 'vendor   '; sha256sum "$tmp/vendor.bin"
printf 'candidate'; sha256sum "$tmp/candidate.bin"
printf 'sizes: vendor=%s candidate=%s\n' "$(wc -c < "$tmp/vendor.bin")" "$(wc -c < "$tmp/candidate.bin")"
if cmp -s "$tmp/vendor.bin" "$tmp/candidate.bin"; then
    echo 'BYTE_EXACT=yes'
else
    echo 'BYTE_EXACT=no'
    cmp -l "$tmp/vendor.bin" "$tmp/candidate.bin" | head -80 || true
fi
