# Explorer Properties

This independently downloadable Seer Canonical v1 Control package opens the
Windows Properties dialog for the current regular file. The helper always uses
the fixed non-destructive `properties` Shell verb.

The package interface in `plugin.json` is fixed:

- backend: `process`
- capability: `control`
- file matcher: `${type_file}`
- command: package-relative `shellverb_properties.exe`
- arguments: `--input ${input_file}`

Build and test the standalone package with:

```powershell
cmake -S control/explorer-properties -B control/explorer-properties/build -G "Visual Studio 17 2022" -A x64
cmake --build control/explorer-properties/build --config Release
ctest --test-dir control/explorer-properties/build -C Release --output-on-failure
```

Install the distributable files into one package root, then create the release
archive from that root only:

```powershell
cmake --install control/explorer-properties/build --config Release --prefix control/explorer-properties/dist
Compress-Archive -Path control/explorer-properties/dist/* -DestinationPath control/explorer-properties/explorer-properties-1.0.0.zip
```

The install step validates that the manifest and helper are present under the
same package root and that the fixed Properties contract is intact; it fails if
any of those checks do not pass. Create the archive only from the validated
`dist` directory.

Install the resulting archive through Seer's plugin installation flow. Verify
the following acceptance cases in a fresh Seer profile:

- A regular file opens its Windows Properties dialog.
- A filename containing Unicode characters opens its Properties dialog.
- A filename containing spaces opens its Properties dialog.
- A missing file is rejected without opening a dialog.
- The helper may exit while the Properties dialog remains open.
- Disabling the package prevents the control from running.
- Uninstalling the package removes the control.
- Reinstalling the package restores the control.

The launcher returns success once the system UI is ready. A separate helper
process keeps the UI alive until it closes, so Seer's command timeout does not
limit how long the user can interact with the dialog.

Build from this repository with `control/common` present; its shared launcher
header is compiled into the executable and adds no package runtime dependency.
