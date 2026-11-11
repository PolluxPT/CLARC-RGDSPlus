# Third-party components

The commercial CLARC game is not distributed. The first-launch importer copies GLU and libstdc++ from the owner's own Linux archive; those files are not in this release.

| Component | Source / version | License / notices |
|---|---|---|
| Box64 with Box32 | ARM64 runtime reused byte-for-byte from [PolluxPT/Braid-RGDSPlus](https://github.com/PolluxPT/Braid-RGDSPlus), v1.6 foundation; upstream [ptitSeb/box64](https://github.com/ptitSeb/box64), commit `1a49e470758276d49b2d09a78e199e12951c6dc3` | MIT, `APPS/CLARC_RGDSPlus/licenses/box64.txt` |
| GL4ES | Runtime reused byte-for-byte from the same Braid foundation; [ptitSeb/gl4es](https://github.com/ptitSeb/gl4es), commit `2037865590d410bcd7b8d1f22b841472ed6d39ac` | MIT-style notice, `licenses/gl4es.txt` |
| SDL2 | Supplied by stock firmware, not bundled; tested firmware reports 2.32.10 | zlib notice retained in `licenses/sdl2.txt` |
| libgcc_s.so.1 (i386) | Debian GCC 12 runtime build, imported by the original integration | GCC notices and runtime exception in `licenses/GCC-runtime-copyright.txt`; GPL text in `licenses/GPL-3.txt`; [Debian GCC 12 source](https://sources.debian.org/src/gcc-12/) |
| X11 build headers | Header copies in `Source/headers/X11/` | Original header notices retained |
| Mono.Cecil | Developer build dependency, 0.11.6; not bundled in the release | [MIT](https://github.com/jbevain/cecil/blob/0.11.6/LICENSE.txt) |
| CLARC integration | New bridge, scripts, helper, importer and local patch application | Repository MIT license |

X11 proxy libraries are thin integration stubs, not a bundled X server. Their source depends on the integration bridge. Neither UnityEngine.dll nor the game's Mono runtime is redistributed; both come from the owner's archive.

`assembly.delta` contains a version-specific source-dependent binary transformation. It needs the exact original assembly hash and verifies the output hash; it is not a standalone copy of the game assembly. Rights in the original game remain with its rights holders.

Runtime hashes are recorded in `runtime/install-manifest.json`; upstream changes should be reviewed and retested rather than silently replacing the confirmed binaries.
