# Seer-plugin

#### How to add a plugin

- The extension of the file is not case sensitive.
- Drag the line number to change sequence
- Triggered only check the plug-in type at a time

![plugin](https://raw.githubusercontent.com/ccseer/Seer/master/img/plugins-add.jpg)

#### How to write a plugin

Seer will add two strings after command line when invoking the plugin. 

``` 
1. The front string is the full path to the triggered file, for example, "E:/my_folder/name.ttf".
2. The other string has a full path of the file and there is no file suffix. For example, "C:/Users/COREY/AppData/Local/Temp/Seer/a-random-name". 
```

The plugin will analysis triggered file into the format of one of the built-in Seer suffix, name the resulting file as a-random-name, and save it on the path, “C:/Users/COREY/AppData/Local/Temp/Seer/”.

To illustrate the function, we use the mentioned font file above as an example.

The invocation of Seer is shown as below. 

`C:/Seer/fontpreview.exe C:/triggering-path/FILE.TTF C:/Seer-temp-path/ABCabc123`

**fontpreview.exe would render FILE.TTF to ABCabc123.png**

*In practical use, it looks like these:*

> - fontpreview_py.exe -t "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789"
> - nconvert.exe -out pdf -o \*SEER_OUTPUT_PATH\*.pdf \*SEER_INPUT_PATH\*
> - 1syt.exe




*Source Code*

- fontpreview [Github](https://github.com/ccseer/Seer-plugins/blob/master/font/fontpreview_py.py)
- MS-Office  [Github](https://github.com/ccseer/Seer-plugins/blob/master/ms-office/1syt.py)

Contact: cc.seer@gmail.com





