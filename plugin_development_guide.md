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
Convert plugins run an external executable or script via command line. The plugin parses the source file, converts it to one of the formats Seer natively supports (such as `HTML`, `JSON`, `TXT`, `PNG/JPG`), and Seer loads and renders the output.

#### Workflow:
1. **Match**: The user selects a file and presses the Spacebar. Seer matches the file extension against the configured plugins.
2. **Execute**: Seer launches the plugin executable or script.
3. **Convert**: The plugin reads/parses `${input_file}`, writes the converted content to `${output_file}.<format>` (e.g. `${output_file}.html`), and exits.
4. **Render**: Once the plugin exits, Seer reads and displays `${output_file}.<format>`.

> [!IMPORTANT]
> **Script Execution and Runtimes:**
> - Seer only adapts `.ps1` files to run automatically via PowerShell (`powershell.exe -NoProfile -ExecutionPolicy Bypass -File <script>`).
> - For any other script formats (such as `.py`, `.js`, `.bat`, `.cmd`), Seer does not wrap them. They are launched directly via `QProcess::start(exe, args)`. 
> - Therefore, other scripts **must be packaged as `.exe` files** (e.g., using PyInstaller or ncc) or run via a compiled launcher included in the plugin package. Do not expect Seer to invoke python.exe or node.exe directly.

#### Command Line Placeholders:
- `${input_file}`: The absolute path of the source file to preview.
- `${output_file}`: The target output file path (omit suffix in the placeholder; append the suffix in the arguments array, e.g. `${output_file}.html`).
- `${no_cache}`: Disables reuse/caching of the output file for subsequent previews of the same file, and stores the output in the auto-delete temp directory. The generated file is deleted after all preview windows (including the main window and any separated windows) are completely closed. Note that `${no_cache}` is intercepted by Seer and not passed to the executable.

---

### 2. DLL Plugins (SDK v3)
Native C++/Qt dynamic link libraries (`.dll`) loaded directly by Seer using `QPluginLoader`.

#### Technical Specifications:
- **Language Standard**: C++17
- **Framework**: Qt 6.8 (Do not use Qt 6.9+ APIs)
- **Compiler**: MSVC 2022 (x64)
- **Build System**: CMake 3.22+
- **Minimum App Version**: DLL plugins compiled with SDK v3 must declare `appMinVersion` as `"4.2.9"`.

#### Essential SDK Interfaces (defined in `seer/viewerbase.h`):
1. **`ViewerBase`**: The base class for the widget performing the preview. The plugin's viewer class must inherit from `ViewerBase`.
2. **`ViewerPluginInterface`**: The interface that Seer uses to instantiate the plugin.
   ```cpp
   class ViewerPluginInterface {
   public:
       virtual ~ViewerPluginInterface() = default;
       virtual ViewerBase *createViewer(QWidget *parent = nullptr) = 0;
   };
   ```
3. **`ViewerPluginInterface_iid`**: The interface identifier string defined as `"seer.plugin.interface.preview/3"`.
4. **Qt Macros**: The plugin class implementing `ViewerPluginInterface` must declare:
   - `Q_OBJECT`
   - `Q_INTERFACES(ViewerPluginInterface)`
   - `Q_PLUGIN_METADATA(IID ViewerPluginInterface_iid)`

---

## 📄 `plugin.json` Schema Specification

Every Seer plugin must contain a `plugin.json` file in its root directory. Seer parses this file using strict JSON parsing (`QJsonDocument::fromJson`). 

> [!WARNING]
> **Strict JSON Compliance:**
> The `plugin.json` must be strictly valid JSON. **No comments (`//` or `/* */`) or trailing commas are allowed.** If there is any formatting error, Seer's installation task will fail.

### Field Definitions

| Field Name | Type | Presence | Description |
| :--- | :--- | :--- | :--- |
| `name` | String | **Required** | The user-facing name of the plugin. Cannot be empty. |
| `version` | String | **Required** | Version string (e.g., `"1.0.0"`). Cannot be empty. |
| `type` | String | **Required** | Must be `"convert"` or `"dll"`. |
| `roles` | Array | **Required** | Array of strings representing roles. Must contain `"viewer"`. Cannot be empty. |
| `entry` | String | **Required** | File name of the entry point (e.g., `"MyConverter.exe"`, `"MyScript.ps1"`, or `"MyPlugin.dll"`). Must point to a file that actually exists inside the plugin directory. |
| `formats` | Array | **Required** | Supported file extensions. Must be **lowercase** and **without a leading dot** (e.g., `["ipynb"]`, NOT `[".ipynb"]`). Cannot be empty. |
| `appMinVersion` | String | **Required** | Must be a version string of at least `"3.9.10"` (e.g., `"4.1.3"` for Convert; `"4.2.9"` for SDK v3 DLL plugins). If missing or below `"3.9.10"`, Seer routes the manifest through the incompatible legacy parser. |
| `args` | Array | **Convert-Specific** | Command line arguments array. Mandatory for `"convert"` plugins (must not be empty). For `"dll"` plugins, it is optional and acts as plugin configuration (passed to the DLL via `ViewOptionsKeys::kKeyPluginCmd`). |
| `author` | String | Optional | Developer's name. |
| `releaseDate` | String | Optional | Release date in YYYY-MM-DD format. |

---

## 📋 `plugin.json` Examples

### Convert Plugin Example
```json
{
    "name": "ipynb",
    "version": "1.2.0",
    "type": "convert",
    "roles": [
        "viewer"
    ],
    "entry": "ipynb.ps1",
    "args": [
        "-i",
        "${input_file}",
        "-o",
        "${output_file}.html"
    ],
    "formats": [
        "ipynb"
    ],
    "appMinVersion": "4.1.3",
    "author": "yourname",
    "releaseDate": "2026-07-14"
}
```

### DLL Plugin Example (SDK v3)
```json
{
    "name": "JsonTreeViewer",
    "version": "1.0.0",
    "type": "dll",
    "roles": [
        "viewer"
    ],
    "entry": "JsonTreeViewer.dll",
    "args": [
        "--expand-depth",
        "2"
    ],
    "formats": [
        "json"
    ],
    "appMinVersion": "4.2.9",
    "author": "yourname",
    "releaseDate": "2026-07-14"
}
```

---

## 🤖 Validation, Packaging & Safety Checklist

Before releasing or loading the plugin, the AI Agent must verify the following items:

### 1. Strict Metadata Verification
- Parse the `plugin.json` through a strict JSON parser (e.g., python's `json.loads` or a JSON validator) to ensure no comments or trailing commas exist.
- Ensure the `entry` file name matches the actual compiled binary or script file name exactly.
- Confirm all entries in the `formats` array are lowercase and do not contain leading dots.

### 2. Sandbox Testing
Test the script or program locally in PowerShell using the exact argument format:
```powershell
# For PowerShell script plugins:
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\ipynb.ps1 -i "test_input.ipynb" -o "output_path.html"

# For executable plugins:
.\MyConverter.exe -i "test_input.xyz" -o "output_path.html"
```
Ensure the program exits with code `0` and generates the expected output format.

### 3. Packaging & Archive Layout
Seer installer behavior differs depending on installation method:
- **Local Installation**: Extract the plugin files locally. Open **Seer Settings -> Plugins**, click **Add from local**, and select the extracted `plugin.json` file.
- **Online/Remote Catalog Installation (ZIP)**: The online package installer expects extraction to create a top-level directory named after the ZIP archive itself. Thus, the ZIP layout must be:
  `<archive-name>/<plugin.json, entry, assets, ...>`
  Do NOT place `plugin.json` directly at the archive root for online catalog packaging.

### 4. Dependency Declaration
- If the plugin relies on external runtimes (such as Python or Node.js), clearly state the requirements in a `README.md`.
- Prefer bundling the runtimes or compiling to a single-file executable (EXE) so the end user does not need to install external development stacks.

### 5. HTML Content Safety & Sanitization
- If your plugin converts content into `HTML` for rendering, you **must escape or sanitize any untrusted input data** extracted from the source files before embedding it into the HTML template to prevent potential script injection vulnerabilities.
