# Developer notes

These instructions are for maintainers. Players install the release and copy their own archive; no developer tools run on the RGDS+.

## Managed integration

Use the original Linux game's .NET 2 `mscorlib.dll` and `UnityEngine.dll` as references. Build `Managed/RGDS.cs` with Roslyn `csc.dll`, `-target:library -nostdlib`, those two explicit references, and output name `ClarcRGDS.dll`. Do not target a modern .NET game runtime.

The host build tools `Patcher/` and `ControlsPatcher/` target .NET 8 and restore Mono.Cecil 0.11.6. They are build tools, not device runtime dependencies.

1. Build both projects.
2. Run Patcher with arguments: original Managed directory, new helper DLL, intermediate Assembly-CSharp.dll. It prefixes 122 startup callbacks; its volume-setter hook count for this release is zero.
3. Put that intermediate assembly in a temporary Managed directory alongside the game's unchanged reference assemblies.
4. Run ControlsPatcher with arguments: intermediate Managed directory, new helper DLL, final Assembly-CSharp.dll. Expected counts: 2 axis returns, 6 button returns, 84 raw key calls.
5. Generate the public delta with `python3 generate_delta.py original-Assembly-CSharp.dll final-Assembly-CSharp.dll assembly.delta`.
6. Update game-import and installation manifests when rebuilding binaries. Never distribute either game assembly.

The original supported assembly SHA-256 is `76ea9b8abafdf206029cf331de3454dac3150a7a42d241367002669bdb47d75b`. The import manifest records the shipped patched assembly and helper hashes. The importer verifies both the original source and reconstructed result.

## Native bridge

`build-native.sh` documents the x86 glibc 2.17 cross-build recipe with Zig, the bundled X11 headers, SDL2 imports and libdl. The shipped bridge is the owner-confirmed Alpha 1.2 context-lifetime/ARB build; the selector is a tiny original replacement for the desktop GTK chooser. Native rebuilds are not automatically interchangeable and need regression testing.

The SDL stub generated during this build is only a link-time import library. Do not include it in a release. At runtime Box32 wraps the firmware's ARM64 SDL2. The X11 proxies retain a dependency on the bridge.

Keep the pinned Box32 and GL4ES binaries unless testing an intentional runtime upgrade. Their source projects, commits and licenses are recorded in THIRD_PARTY.md.

## Local verification

- `Tests/device_checks.py` exercises the captured device capability bitmap and physical Chasm calibration, all L1/R1/SELECT/START combinations, D-pad directions, releases and the real exit-chord branch. It produces packets.json.
- Build `Tests/ManagedChecks.csproj`. Run its output with arguments: shipped ClarcRGDS.dll, reconstructed Assembly-CSharp.dll, packets.json. The release passes 606 mapping/IL checks.
- `Tests/test_import.py` checks the local delta decoder and file-import failure handling.
- Check the shell launcher with `sh -n`. Verify a complete extracted release after first-run preparation with runtime/verify-install.py.

Game and reference assemblies needed for a developer build are private local inputs. Keep them out of the repository.
