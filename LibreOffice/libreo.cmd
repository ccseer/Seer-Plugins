@echo off
:: ukj@ukj.ee, Script for Seer

for /r %%f in (%1) do (
Set TempBasename=%%~nf
)

IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Users\i7\Programmid\LibreOfficePortable\App\libreoffice\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Program Files (x86)\OpenOffice.org 3\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Program Files (x86)\OpenOffice 4\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Program Files (x86)\LibreOffice 4\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Program Files (x86)\LibreOffice 5\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV
SET OfficePyRun="C:\Program Files (x86)\LibreOfficePortable\App\libreoffice\program\python.exe"
IF EXIST %OfficePyRun% GOTO UNOCONV

:UNOCONV
@%OfficePyRun% "C:\Program Files (x86)\Seer\plugins\LibreOffice\unoconv.py" -e PageRange=1,2,3,4,5 -f pdf -o "%2.pdf" %1

Set TempBasename=
exit /B 0
