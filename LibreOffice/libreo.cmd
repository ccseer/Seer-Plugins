@echo off
:: /* Uku-Kaarel Jõesaar
:: ukjoesaar@gmail.com
:: v1.0 */

for /r %%f in (%1) do (
Set TempBasename=%%~nf
)


Set OfficeRun="C:\Program Files\LibreOffice 5\program\soffice.exe"

@%OfficeRun% -headless -convert-to pdf -outdir %TEMP% %1
@move /Y "%TEMP%\%TempBasename%.pdf" "%2.pdf"
Set TempBasename=

exit /B 0