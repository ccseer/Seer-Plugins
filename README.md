## How to write a plugin

### 1. Convert unknown type to built-in type

##### Seer takes the first argument as the executor and then replaces the placeholders. `*SEER_INPUT_PATH*` is the absolute file path you just triggered. `*SEER_OUTPUT_PATH*` is the temp file path used to save converted file.

```batch
"C:/ApkMetaInfo2Json/ApkMetaInfo2Json.exe" "*SEER_INPUT_PATH*" "*SEER_OUTPUT_PATH*.json"
```

- process

  1. A file is triggered by **SPACE**
  2. Seer searches from plugin settings and finds that `apk` is defined as a plugin type
  3. Seer runs `ApkMetaInfo2Json.exe` with defined parameters
     - In this case, `ApkMetaInfo2Json.exe` read contents from the triggered `apk` file, then write a `json` file at `*SEER_OUTPUT_PATH*`
  4. When the `ApkMetaInfo2Json.exe` finishes and exits, Seer will go to the temporary directory to read the `json` file and display it.

- A plug-in can be any executable program

  - a `CMD` script: [rename](https://github.com/ccseer/Seer-plugins/tree/master/rename)
  - a `BAT` script: [epub](https://github.com/ccseer/Seer-plugins/tree/master/epub)
  - python: [Font](https://github.com/ccseer/Seer-plugins/tree/master/font)
    - the command line should be something like this
      ```batch
      \"C:/path/to/your/python.exe\" your_python_script_file_path "*SEER_INPUT_PATH*" "*SEER_OUTPUT_PATH*
      ```
  - any third-party program that provides conversion functionality
    - `dll_lib_exports`: this is extracted somewhere from Microsoft Windows
    - `ImageMagick`: https://imagemagick.org/index.php
    - `exiftool`: https://exiftool.org/
  - any user-defined program:
    - [Torrent2Json](https://github.com/ccseer/Seer-plugins/tree/master/Qt_Torrent2Json)
    - [ApkMetaInfo2Json](https://github.com/ccseer/Seer-plugins/tree/master/Qt_ApkMetaInfo2Json)

- Usually such plugins generate a temporary file.
  - Seer will automatically delete temporary files older than 20 days.
  - Temporary files will be removed by adding the parameter `*SEER_NO_CACHE*` at the end of the commands.

### 2. Internal Support

##### This kind of plugin can customize the display instead of the built-in display mode of the Seer.

- It needs to be written by Qt/C++.
- The example code can be found here: [FontViewer](https://github.com/ccseer/FontViewer)

---

- How to add plugin
  [http://1218.io/seer/plug-in.html](http://1218.io/seer/plug-in.html?github)
