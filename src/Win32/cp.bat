@echo off
:loop
if "%1" == "-f" shift
if "%1" == "." shift
if "%1" == ".." shift
if "%1x" == "x" goto end
copy %1
shift
goto loop
:end
