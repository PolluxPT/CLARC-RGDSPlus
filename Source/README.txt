CLARC v1.0.0 integration source (MIT for new integration files).
X11 headers retain their original license notices. Existing runtime
licenses are under APPS/CLARC_RGDSPlus/licenses.

bridge.c is an i386 X11/GLX compatibility library backed by wrapped SDL2.
It is not an X server. All directly imported Unity X11/GLX names were audited.
The SDL import-only build library is a link-time artifact and is NOT shipped.
For production, link the bridge against an SDL2 import library with SONAME
libSDL2-2.0.so.0, use -target x86-linux-gnu.2.17, PIC and libdl. The runtime
Box32 wraps that SDL dependency to the firmware ARM64 SDL2.
X11 proxy libraries depend on the bridge; they contain no Xorg code.

Managed/RGDS.cs targets the game's own .NET 2-compatible mscorlib and
UnityEngine.dll via csc -nostdlib. Patcher injects initialization into the
owner's Assembly-CSharp Awake/Start/OnEnable callbacks. The helper handles
volume, a quality/frame-rate target and graceful exit requests. Inputs
are mapped through a small shared-memory mailbox, not per-frame SD writes.

The Box32 and GL4ES binaries are the pinned public Braid RGDS+ v1.6 runtime
from PolluxPT. That compatibility foundation is known to run on this device;
the owner has confirmed this CLARC build runs with corrected physical controls.
