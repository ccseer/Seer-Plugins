# Seer Plugins

Resources and reference implementations for extending [Seer](https://1218.io/) with custom file-preview plugins.

## Build a Plugin with an AI Agent

Replace the requirement placeholder below, then give the complete instruction to your coding agent:

```text
Read and follow the Seer plugin development guide at https://raw.githubusercontent.com/ccseer/Seer-Plugins/refs/heads/master/plugin_development_guide.md. Treat it as the source of truth for choosing between Convert and DLL plugins, defining plugin.json, validating the implementation, and packaging the result.

Before doing any implementation work, inspect the Requirement section at the end of this instruction. If it is missing, blank, still contains placeholder text, or does not identify the target file format and desired preview behavior clearly enough to act on, ask exactly one clarifying question and wait for the answer. Do not infer a requirement, choose a plugin type, or create files until a concrete request is provided.

Once the requirement is clear, implement a complete, installable Seer plugin for it. Work in the current workspace and create the plugin in a new, appropriately named directory. Do not modify Seer itself or stop at a plan or isolated code snippets.

Before implementation, choose Convert or DLL and briefly justify the choice. Then create every required source, manifest, asset, sample, and documentation file. Validate plugin.json with a strict JSON parser, build or run the plugin against a representative sample, verify the generated preview or DLL build, and report the exact verification results. Also document packaging, local installation, external runtime requirements, and any dependency that must be installed separately.

If any essential context remains ambiguous after a concrete requirement is provided, ask one clarifying question before proceeding. Otherwise, continue until the plugin and its focused verification are complete.

Requirement: [REPLACE THIS PLACEHOLDER with the file format, desired preview behavior, sample files or format specification, and any implementation constraints.]
```

## Resources

- [Add a plugin to Seer](https://1218.io/docs/seer/add-plugin)
- [Plugin development guide](./plugin_development_guide.md)
- [Official plugin development documentation](https://1218.io/docs/seer/create-plugin)
- [Seer SDK](https://github.com/ccseer/Seer-sdk)
- [Existing plugin examples](https://github.com/stars/ccseer/lists/plugins)
