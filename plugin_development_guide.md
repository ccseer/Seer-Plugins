# Seer Plugin Development Specification & Reference Guide

This guide is designed to instruct AI Agents on how to develop and write custom plugins for Seer, the Windows Quick Look tool.

---

## 🔗 Reference Links

- **Official Plugin Development Docs**: [https://1218.io/docs/seer/create-plugin.html](https://1218.io/docs/seer/create-plugin.html)
- **Existing Plugins List (for reference)**: [https://github.com/stars/ccseer/lists/plugins](https://github.com/stars/ccseer/lists/plugins)
- **Seer SDK Repository**: [https://github.com/ccseer/Seer-sdk](https://github.com/ccseer/Seer-sdk)

---

## 🛠️ Plugin Architecture Overview

Seer supports two types of plugins to extend file preview capabilities: **Convert Plugins** and **DLL Plugins**.

### 1. Convert Plugins

Convert plugins run an external program or script (such as `.exe`, `.bat`, `.cmd`, `.ps1`, `python`, etc.) via command line. The plugin parses the source file, converts it to one of the formats Seer natively supports (such as `HTML`, `JSON`, `TXT`, `PNG/JPG`), and then Seer loads and renders the output.

#### Workflow:

1. **Match**: The user selects a file and presses the Spacebar. Seer matches the file extension against the configured plugins.
2. **Execute**: Seer launches the plugin executable or script, replacing the command line placeholders:
   - `${input_file}`: Replaced with the absolute path of the source file (e.g., `C:\path\to\file.apk`).
   - `${output_file}`: Replaced with a target file path in Seer's temp folder named after the file's MD5 (without extension).
3. **Convert**: The plugin reads/parses `${input_file}`, writes the converted content to `${output_file}.<format>` (e.g. `${output_file}.html`), and exits.
4. **Render**: Once the plugin exits, Seer reads and displays `${output_file}.<format>`.

#### Command Line Placeholders:

- `${input_file}`: The source file path.
- `${output_file}`: The target output file path (omit suffix in the placeholder itself; specify the suffix explicitly in the arguments array).
- `${no_cache}`: If included in the arguments list, Seer will automatically delete the generated temporary file after the preview window is closed (useful for frequently modified file formats). Note that `${no_cache}` is intercepted by Seer and not passed to the executable.

#### `plugin.json` Configuration Structure:

Every convert plugin directory must contain a `plugin.json` file declaring the plugin attributes. Example:

```json
{
  "name": "ipynb", // Name of the plugin
  "version": "1.2.0", // Plugin version
  "type": "convert", // Plugin type: must be "convert"
  "roles": ["viewer"], // Role: must be ["viewer"]
  "entry": "ipynb.ps1", // Entry script/executable file path
  "args": [
    // Command line arguments list passed to the entry file
    "-i",
    "${input_file}",
    "-o",
    "${output_file}.html" // Target extension helps Seer choose the correct built-in viewer
  ],
  "formats": ["ipynb"], // Supported file extensions list
  "appMinVersion": "4.1.3", // Minimum Seer version required
  "author": "yourname", // Author
  "releaseDate": "2026-07-14" // Release date (YYYY-MM-DD)
}
```

---

### 2. DLL Plugins

Native C++/Qt dynamic link libraries (`.dll`) loaded directly by Seer.

#### Technical Specifications:

- **Language Standard**: C++17
- **Framework**: Qt 6.8 (do not use Qt 6.9+ APIs)
- **Compiler**: MSVC 2022 (x64)
- **Build System**: CMake 3.22+
- **Benefits**: Runs within Seer's main process, shares its runtime dependencies (Qt, etc.), resulting in faster load times and smaller package sizes.

#### Reference Projects:

- [F3DViewer](https://github.com/ccseer/f3dviewer) - 3D file viewer plugin
- [OfficeViewer](https://github.com/ccseer/OfficeViewer) - Office document viewer plugin
- [FontViewer](https://github.com/ccseer/FontViewer) - Font preview plugin
- [JsonTreeViewer](https://github.com/ccseer/JsonTreeViewer) - JSON structure viewer plugin

---

## 🤖 AI Agent Implementation Steps

When you (the AI Agent) receive a task to create a new plugin for Seer, follow these steps:

1. **Understand Target Format**:
   - Analyze the structure of the target file extension.
   - Decide the best output format for previewing (e.g. structured data is best rendered as `.html` or `.json`; plain text should be `.txt`).

2. **Choose Plugin Type**:
   - Prefer **Convert Plugins** using scripts (Python, PowerShell, Node.js) or compiled console binaries for faster development.
   - Make sure your script handles file paths containing spaces and forces `UTF-8` encoding for file reads/writes.

3. **Establish Plugin Folder Structure**:
   Create a new folder in the `Seer-plugins` repository containing:
   - `plugin.json` (metadata, arguments, and extension formats)
   - The conversion script/executable
   - `assets/` or `resources/` (optional: for storing HTML templates, CSS/JS styling resources)

4. **Implement Conversion Logic**:
   - Parse `${input_file}` and write to `${output_file}.<suffix>`.
   - Write clear error descriptions to the output file if conversion fails, so the user knows what went wrong instead of seeing a blank window.

5. **Test Individually**:
   - Verify the command-line execution manually:
     `python convert.py "test_input.ext" "output_path.html"`
