# Seer Plug-in

#### How to add a plugin

- The extension of file is case insensitive
- Drag line number to change sequence
- Triggered only check the plug-in type once

To install a plugin, select it in the `Settings` and hit the `Download` button afterwards. This initiates the download of the plugin file in the default system browser.

All plugins are provided as zip archives which you need to extract on the local system. Afterwards, switch to the `Local` tab under `Plugins` in Seer's `Settings`, and click on add. This opens a file browser which you use to select the `json` file in the folder you have extracted the archive to.

Once done, support for the new file types has been added to Seer.



#### How to write a plugin

######Replace invocation 

Seer replaces `*SEER_INPUT_PATH*` with the triggered file, replaces `*SEER_OUTPUT_PATH*` with the saving path. If  the plugin output file doesn't contain a suffix, you can append suffix to the `*SEER_OUTPUT_PATH*`.

*In practical use, it looks like this:*

> nconvert.exe -out pdf -o \*SEER_OUTPUT_PATH\*.pdf \*SEER_INPUT_PATH\*



*Source Code for example*

- [fontpreview](https://github.com/ccseer/Seer-plugins/blob/master/font/fontpreview_py.py)
- [MS-Office](https://github.com/ccseer/Seer-plugins/blob/master/ms-office/1syt.py)





