#!/bin/sh
# Developer build only. Requires zig on PATH; no game assets are needed.
set -eu
cd "$(dirname "$0")"
mkdir -p build-native
python3 - <<'PY'
import pathlib,re
symbols=sorted(set(re.findall(r'extern[^;]*\b(SDL_[A-Za-z0-9_]+)\s*\(',pathlib.Path('bridge.c').read_text())))
pathlib.Path('build-native/sdl-import.c').write_text('\n'.join('void '+n+'(void) {}' for n in symbols)+'\n')
pathlib.Path('build-native/proxy.c').write_text('extern void clarc_bridge_marker(void); void *clarc_x11_anchor = (void *)clarc_bridge_marker;\n')
PY
zig cc -target x86-linux-gnu.2.17 -shared -fPIC -Wl,-soname,libSDL2-2.0.so.0 build-native/sdl-import.c -o build-native/libSDL2-2.0.so.0
zig cc -target x86-linux-gnu.2.17 -shared -fPIC -O2 -Iheaders bridge.c -Lbuild-native -l:libSDL2-2.0.so.0 -ldl -Wl,-soname,libclarc_wayland.so -o build-native/libclarc_wayland.so
for name in libX11.so.6 libXext.so.6 libXcursor.so.1; do
 zig cc -target x86-linux-gnu.2.17 -shared -fPIC -Os build-native/proxy.c -Lbuild-native -lclarc_wayland -Wl,-soname,"$name" -o "build-native/$name"
done
zig cc -target x86-linux-gnu.2.17 -shared -fPIC -Os selector.c -o build-native/ScreenSelector.so
# libSDL2-2.0.so.0 above is a LINK-TIME IMPORT STUB. NEVER ship it.
