# CLARC for Anbernic RGDS+

Standalone CLARC Linux port for the RGDS+ stock firmware. No PortMaster installation is required.

**Release:** v1.0.0 · **Supported game:** DRM-free Linux `clarc_1.0.0.3.tar.gz`.

The owner confirmed that the game runs and the corrected physical controls work on the RGDS+. The initial release does not claim a complete playthrough of every level.

## What you need

- Anbernic RGDS+ with the tested stock Linux environment, RK3568 ARM64 and 1 GB RAM.
- Your own legally obtained DRM-free **Linux** version of CLARC, archive name `clarc_1.0.0.3.tar.gz`.
- At least **1 GB of free SD-card space**, in addition to the archive itself, for the first installation.

The Windows game, Steam installers and other game versions are not supported by this release.

## Install

1. Download `CLARC_RGDSPlus_v1.0.0.zip` from [Releases](https://github.com/PolluxPT/CLARC-RGDSPlus/releases).
2. Extract it. Copy its **APPS** folder to the root of your SD card. Merge folders and replace files when asked.
3. Copy your own `clarc_1.0.0.3.tar.gz` into **APPS/CLARC_RGDSPlus/**. Keep the archive compressed.
4. Safely eject the card, put it back into the RGDS+ and launch **CLARC** from the handheld's Apps menu.

The first launch prepares the game automatically and can take a few minutes. Let it finish. After one successful launch, you may delete the archive from the card to recover space.

The first launch needs no Internet connection, terminal commands or new button calibration.

## Controls

| Physical control | Action |
|---|---|
| D-pad or left stick | Movement |
| L1 | Robot rotation left, through right-stick left |
| R1 | Robot rotation right, through right-stick right |
| A / X | Interaction / grapple, according to game context |
| B | Back / cancel |
| START | Pause / menu |
| SELECT + START, held 1.5 seconds | Exit to the handheld menu |
| Volume buttons | Adjust volume using the handheld curve |
| L2 / R2 | Unassigned |

The physical button identities use the owner's original Chasm calibration: L1/R1 are buttons 4/5; SELECT/START are buttons 6/7. The D-pad follows the same gameplay movement path as the left stick.

## Audio, screen and saves

The bottom screen is powered off during play. On exit, the launcher restores its prior screen state and the system volume from before the game started. The volume gain follows `0.35 × (level / 10)²`.

Saves and preferences stay inside **APPS/CLARC_RGDSPlus/savedata/**. Keep this folder when updating. Existing confirmed Alpha 1.4 installations are accepted automatically when their game files match the release.

## Troubleshooting

If the game returns to the Apps menu, check that the archive has the exact filename above and enough SD-card space is available. Launch once more after fixing it.

The automatically generated **APPS/CLARC_RGDSPlus/CLARC_TestLogs.zip** contains launcher and game logs for a support report. Do not upload saves or your game archive to a public issue. If game files are damaged after installation, copy the original archive back and launch again for automatic repair.

## How it runs

The original Unity Linux player is **32-bit x86**, translated locally through the bundled ARM64 Box64/Box32 runtime. This release does not depend on Box86 or ARM32 userspace. A small X11/GLX compatibility bridge uses the firmware's SDL2/Wayland display and the bundled GL4ES renderer. It launches at 640×360 with a 30 FPS target and the game's low quality setting; this is a target rather than a guaranteed frame rate in every scene.

The owner supplies all game files. On first launch, a source-dependent delta modifies the owner's game assembly, and the port adds its own control/audio helper and resolution-selector stub. The original archive supplies its own GLU and C++ libraries. No complete game assembly, executable, assets, saves or archive are included here.

## Sources and credits

New integration source is in **Source/**. See [THIRD_PARTY.md](THIRD_PARTY.md) for runtime provenance and licenses, and [Source/BUILD.md](Source/BUILD.md) for developer notes. CLARC and its original content belong to their respective rights holders. This is an unofficial community port; it is not endorsed by the game's publisher or developer.

The repository's MIT license applies to the new integration code. It does not grant rights to the commercial game or replace third-party licenses.
