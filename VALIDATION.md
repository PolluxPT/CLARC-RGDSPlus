# v1.0.0 release validation

- The owner confirmed that the game and corrected controls work on the RGDS+ after Alpha 1.4.
- The shipped game transformation reconstructs the same Assembly-CSharp.dll as that confirmed build, SHA-256 `329468fa80aa15e85fbe43177ae0c73d6aff59b076a44c5420d791f93cef7ae5`.
- Rebuilding the two Cecil patch stages from the original assembly reproduces that file byte-for-byte (122 initialization callbacks, 2 axis returns, 6 button returns, 84 raw key calls).
- The published delta generator reproduces the shipped source-dependent patch byte-for-byte.
- Clean installation from the owner's original Linux archive passed locally; existing-install detection then passed without reimporting.
- Full installed-file size and critical checksum verification passed after the clean import.
- 606 real compiled managed mapping/IL checks passed, including physical-button packets from the Python layer.
- Python fixtures passed for SDL enumeration, the saved Chasm calibration, every L1/R1/SELECT/START combination, D-pad directions, releases and the real hold-to-exit branch.
- Five importer checks passed for unsupported input, invalid patch headers, missing game archives, unsafe archive paths, incomplete archives and preservation of existing saves/files on those failures.
- Shell syntax and host bridge syntax checks passed. Runtime native binaries remain the confirmed versions.
- The new first-launch importer has been tested in the host environment. A fresh SD-card install of the public package has not been separately tested on physical hardware, and a complete playthrough is not claimed.

Public-package audits exclude the original game executable, game assemblies, Unity/Mono libraries, assets, owner-supplied GLU/libstdc++, original archive, saves and logs. The integration helper and native bridge/selector stubs are original port components, separate from those commercial files.
