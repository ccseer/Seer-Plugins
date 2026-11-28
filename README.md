# Seer Plugins

Resources and reference implementations for extending [Seer](https://1218.io/) with custom file-preview plugins.

## Build a Plugin with an AI Agent

The [Plugin Development](https://1218.io/docs/seer/create-plugin.html#viewers) page on the documentation site carries the prompt to give your coding agent. It points at [Viewer_Plugin_PROMPT.md](https://github.com/ccseer/Seer-sdk/blob/main/Viewer_Plugin_PROMPT.md), the development guide for both Convert and DLL (SDK v3) plugins.

## Samples

This repository keeps the original Convert plugin samples. They predate the current guide and use heterogeneous manifests, so treat them as working examples rather than templates:

- `ps1_ipynb/`, `ps1_epub/` — PowerShell-driven converters.
- `bat_epub/`, `cmd_rename/` — batch and command-script converters.
- `Qt_ApkMetaInfo2Json/`, `Qt_Torrent2Json/` — C++/Qt converters producing JSON.

For current DLL (SDK v3) plugins, see the reference projects listed on the [documentation page](https://1218.io/docs/seer/create-plugin.html#viewers).

## Resources

- [Add a plugin to Seer](https://1218.io/docs/seer/add-plugin.html)
- [Viewer plugin development guide](https://github.com/ccseer/Seer-sdk/blob/main/Viewer_Plugin_PROMPT.md)
- [Official plugin development documentation](https://1218.io/docs/seer/create-plugin.html)
- [Seer Controls](https://github.com/ccseer/Seer-Controls)
- [Seer Properties](https://github.com/ccseer/Seer-Properties)
- [Seer SDK](https://github.com/ccseer/Seer-sdk)
- [Existing plugin examples](https://github.com/stars/ccseer/lists/plugins)
