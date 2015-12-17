@echo off
for /r %%f in (%1) do (
Set TempBasename=%%~nf
)

IF EXIST "C:\Program Files (x86)\OpenOffice.org 3\program\python.exe" SET OfficePyRun=C:\Program Files (x86)\LibreOffice 5\program\python.exe"
IF EXIST "C:\Program Files (x86)\OpenOffice 4\program\python.exe" SET OfficePyRun=C:\Program Files (x86)\LibreOffice 5\program\python.exe"
IF EXIST "C:\Program Files (x86)\LibreOffice 4\program\python.exe" SET OfficePyRun=C:\Program Files (x86)\LibreOffice 5\program\python.exe"
IF EXIST "C:\Program Files (x86)\LibreOffice 5\program\python.exe" SET OfficePyRun=C:\Program Files (x86)\LibreOffice 5\program\python.exe"
IF EXIST "C:\Program Files (x86)\LibreOfficePortable\App\libreoffice\program\python.exe" SET OfficePyRun="C:\Program Files (x86)\LibreOfficePortable\App\libreoffice\program\python.exe"

@%OfficePyRun% "C:\Program Files (x86)\Seer\plugins\LibreOffice\unoconv.py" -e PageRange=1,2,3,4,5,6,7,8,9 -f pdf -o "%2.pdf" %1

Set TempBasename=

exit /B 0