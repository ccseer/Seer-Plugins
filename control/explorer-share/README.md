# Explorer Share Control

This is a Canonical v1 Control package for Seer. It opens the Windows system
Share UI for the current regular file using Windows Runtime DataTransferManager.

The package interface is defined by `plugin.json`:

- backend: `process`
- capability: `control`
- file matcher: `${type_file}`
- command: package-relative `seer_share.exe`
- arguments: `--input ${input_file}`

Build and validate the package with:

```powershell
$cppWinRt = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\Include" -Directory | Sort-Object Name -Descending | ForEach-Object { $candidate = Join-Path $_.FullName "cppwinrt"; if (Test-Path (Join-Path $candidate "winrt\base.h")) { $candidate; break } }
cmake -S control/explorer-share -B control/explorer-share/build -G "Visual Studio 17 2022" -A x64 -DCPPWINRT_INCLUDE_DIR="$cppWinRt"
cmake --build control/explorer-share/build --config Release
ctest --test-dir control/explorer-share/build -C Release --output-on-failure
```

Create a distributable package with:

```powershell
cmake --install control/explorer-share/build --config Release --prefix control/explorer-share/dist
Compress-Archive -Path control/explorer-share/dist/* -DestinationPath control/explorer-share/explorer-share-1.0.0.zip
```

The manifest test stages the CMake-built `seer_share.exe` with a
CMake-controlled copy of `plugin.json`. It verifies the package contract and
path containment without requiring an installed release executable.

The launcher returns success once the system UI is ready. A separate helper
process keeps the UI alive until it closes, so Seer's command timeout does not
limit how long the user can interact with the dialog.

Build from this repository with `control/common` present; its shared launcher
header is compiled into the executable and adds no package runtime dependency.
