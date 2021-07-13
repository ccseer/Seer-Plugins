echo %0 %1 %2


set target=%1
set target=%target:\=/% 

echo %~dp2 %~nx2 %target%

xcopy /F /S /Y "./src/*" "%~dp2"

set "search=@PLACEHOLDER_FILEPATH@"
set "replace=%target%"


call replace.vbs "%~dp2epub_index.html" "%search%" "%replace%"

ren "%~dp2epub_index.html" "%~nx2"