Developer-only local checks (not required to install or play):
1. Run device_checks.py with Python 3 to create packets.json next to it.
2. Build ManagedChecks.csproj with .NET 8.
3. Run ManagedChecks.dll with arguments: ClarcRGDS.dll, Assembly-CSharp.dll,
   packets.json. Use the two installed DLLs from the game Managed directory.
The helper DLL itself targets the original Unity 4/.NET 2 game, not .NET 8.
