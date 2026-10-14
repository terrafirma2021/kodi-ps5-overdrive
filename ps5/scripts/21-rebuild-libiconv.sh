#!/usr/bin/env bash
# Build the PS5 SDK's GNU libiconv with its extra encodings. Kodi's ZIP
# reader uses iconv("CP437", "UTF-8") for filenames without the UTF-8 flag.
set -euo pipefail

export PS5_PAYLOAD_SDK="${PS5_PAYLOAD_SDK:-/opt/ps5-payload-sdk}"
WORK="${WORK:-$HOME/ps5-work}"
TARBALL="$WORK/pacbrew-repo/libiconv/libiconv-1.17.tar.gz"
LIBRARY="$PS5_PAYLOAD_SDK/target/user/homebrew/lib/libiconv.a"
NM="$PS5_PAYLOAD_SDK/bin/llvm-nm"

[[ -f "$TARBALL" && -f "$LIBRARY" && -x "$NM" ]] || {
  echo "Missing libiconv source, SDK library, or llvm-nm" >&2
  exit 1
}

# Use a fresh source tree. GNU configure's cached results from a previous
# build can silently retain the SDK package's reduced encoding set.
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf -- "$BUILD_DIR"' EXIT
tar -xzf "$TARBALL" -C "$BUILD_DIR"
source "$PS5_PAYLOAD_SDK/toolchain/prospero.sh"
cd "$BUILD_DIR/libiconv-1.17"
./configure --prefix="$PS5_HBROOT" --host=x86_64-pc-freebsd \
  --enable-static --disable-shared --enable-extra-encodings
make -j"${JOBS:-8}"

CANDIDATE="$PWD/lib/.libs/libiconv.a"
[[ -s "$CANDIDATE" ]] || { echo "libiconv build produced no archive" >&2; exit 1; }

# Verify the archive before replacing the SDK copy. An alias string alone
# does not prove that CP437 conversion was built.
"$NM" "$CANDIDATE" > "$BUILD_DIR/symbols.txt"
grep -q 'cp437_mbtowc' "$BUILD_DIR/symbols.txt" || {
  echo "Built libiconv still lacks the CP437 converter" >&2
  exit 1
}

[[ -e "$LIBRARY.before-extra-encodings" ]] || \
  cp -p "$LIBRARY" "$LIBRARY.before-extra-encodings"
install -m 644 "$CANDIDATE" "$LIBRARY"
"$NM" "$LIBRARY" > "$BUILD_DIR/installed-symbols.txt"
grep -q 'cp437_mbtowc' "$BUILD_DIR/installed-symbols.txt" || {
  echo "Installed libiconv lacks CP437" >&2
  exit 1
}

echo "PS5 libiconv now includes CP437. Relink Kodi to use it."
. "$(dirname "${BASH_SOURCE[0]}")/lib/sysroot-changed.sh"
sysroot_changed
