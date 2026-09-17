# RGB Histogram Property Plugin

This standalone package is a Canonical v1 process plugin that computes RGB
histograms and exposure statistics for supported image formats. Its manifest
declares a Canonical Property invocation using `result_schema: 1` and produces
both exposure summary metrics and an attached rendered histogram graph
(`rgb-histogram.png`).

The package interface is defined by `plugin.json`:

- backend: `process`
- capability: `property`
- extensions: `png`, `jpg`, `jpeg`, `bmp`, `webp`, `tif`, `tiff`
- command: package-relative `image_histogram.exe`
- arguments: `--input ${input_file} --output ${output_file} --output-dir ${output_dir}`
- output: `${output_file}.json` with `result_schema: 1`, plus `${output_dir}/rgb-histogram.png`
- Y-axis scale: `log1p`

Set `QTDIR` to your Qt 6.8 MSVC x64 installation directory, then run these
commands from the repository root:

```powershell
cmake -S property/image-histogram-property -B property/image-histogram-property/build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="$env:QTDIR"
cmake --build property/image-histogram-property/build --config Release
ctest --test-dir property/image-histogram-property/build -C Release --output-on-failure
```

Create a distributable package with:

```powershell
cmake --install property/image-histogram-property/build --config Release --prefix property/image-histogram-property/dist
& "$env:QTDIR/bin/windeployqt.exe" --release --no-translations --no-system-d3d-compiler --dir property/image-histogram-property/dist property/image-histogram-property/dist/image_histogram.exe
Compress-Archive -Path property/image-histogram-property/dist/* -DestinationPath property/image-histogram-property/image-histogram-property-1.0.0.zip
```
