@echo off
:loop
if "%1x" == "x" goto end
if "%1" == "-f" shift
del %1 >nul
shift
goto loop
:end
