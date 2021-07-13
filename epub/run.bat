echo %0 %1 %2

if [%1]==[] (
    echo "empty arg1" 
    GOTO  :EOF
)
if [%2]==[] (
    echo "empty arg2" 
    GOTO  :EOF
)

set target=%1
set target=%target:\=/% 

echo %~dp2, %~nx2, %target%,  "%~dp2%~n2.epub"

xcopy /F /S /Y "./src/*" "%~dp2"
@REM append _ to the end, otherwise there will be two files with same md5 and different suffix
copy  /Y %1 "%~dp2%~n2_.epub"

set "search=@PLACEHOLDER_FILEPATH@"
set "replace=%target%"


call replace.vbs "%~dp2epub_index.html" "%search%" "%~n2_.epub"

move /y "%~dp2epub_index.html" "%~dp2%~nx2"

