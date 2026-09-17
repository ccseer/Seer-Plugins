# SHA-256 Property

This standalone package is a Canonical v1 process plugin that computes a
SHA-256 property for every regular file, including extensionless and Unicode
paths. Its manifest uses the host `${type_file}` matcher and writes a flat JSON
property file beside the requested output base path.

The package interface is fixed by `plugin.json`:

- backend: `process`
- capability: `property`
- file matcher: `${type_file}`
- command: package-relative `sha256_property.exe`
- arguments: `--input ${input_file} --output ${output_file}` (optional: `--case lower|upper`)
- output: `${output_file}.json`, containing `{"SHA-256":"<64 hex characters>"}` (lowercase by default)

### Output Case Customization

The helper executable accepts an optional `--case lower|upper` argument (defaulting to `lower`).
To configure uppercase hash output in Seer's plugin settings, override the capability arguments with:

```json
["--input", "${input_file}", "--output", "${output_file}", "--case", "upper"]
```

Build and validate the package with:

```powershell
cmake -S property/sha256-property -B property/sha256-property/build -G "Visual Studio 17 2022" -A x64
cmake --build property/sha256-property/build --config Release --target sha256_property_manifest_test
ctest --test-dir property/sha256-property/build -C Release -R sha256_property_manifest_test --output-on-failure
```

Create a distributable package with:

```powershell
cmake --install property/sha256-property/build --config Release --prefix property/sha256-property/dist
Compress-Archive -Path property/sha256-property/dist/* -DestinationPath property/sha256-property/sha256-property-1.0.0.zip
```

The manifest test stages both `plugin.json` and the helper, checks the exact
manifest tokens and package-relative command, then invokes the staged helper
to verify that a `base` output produces `base.json`.
