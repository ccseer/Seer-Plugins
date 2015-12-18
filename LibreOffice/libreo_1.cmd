@echo off
for /r %%f in (%1) do (
Set TempBasename=%%~nf
)

Set OfficeRun="C:\Program Files (x86)\LibreOffice 5\program\soffice.exe"

:: -env:"UserInstallation=%TEMP%'"
@%OfficeRun% -headless -convert-to pdf -outdir %TEMP% %1
@move /Y "%TEMP%\%TempBasename%.pdf" "%2.pdf"
Set TempBasename=

exit /B 0