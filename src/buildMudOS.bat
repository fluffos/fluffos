@echo off

if "%1" == "develop" echo Preparing to build developmental version of MudOS driver ...
if "%1" == "develop" set OPTIMIZE=-Od
if "%1" == "develop" set DEBUG=-Zi -DDEBUG -W4
if "%1" == "debug" echo Preparing to build debugging version of MudOS driver ...
if "%1" == "debug" set OPTIMIZE=-Od
if "%1" == "debug" set DEBUG=-Zi -DDEBUG -W2
if "%1x" == "x" echo Preparing to build standard MudOS driver ...
if "%1x" == "x" set OPTIMIZE=-Ox

rem  define this to be where you want the temporary compiled files to go
rem  (use only backslashes here)
set OBJDIR=.\obj

rem  change this if you wish the driver binary to be named something else
set DRIVER_BIN=driver

rem Set INSTALL_DIR to the directory where you want to install the executables.
rem (use only backslashes here)
set INSTALL_DIR=..\bin

rem Some of these are probably unnecessary
set CFLAGS=-nologo -MT %DEBUG%

rem Determine system type
set ARCH=Windows 95
if "%OS%" == "Windows_NT" set ARCH=Windows NT
if "%OS%" == "Windows_NT" set CFLAGS=%CFLAGS% -DWINNT
if "%OS%" == "" set CFLAGS=%CFLAGS% -DWIN95

mkdir %OBJDIR%

echo MAKE=NMake >Makefile.tmp
echo SHELL=command.com >>Makefile.tmp
echo OBJDIR=%OBJDIR% >>Makefile.tmp
echo DRIVER_BIN=%DRIVER_BIN% >>Makefile.tmp
echo INSTALL=copy >>Makefile.tmp
echo INSTALL_DIR=%INSTALL_DIR% >>Makefile.tmp
echo OPTIMIZE=%OPTIMIZE% >>Makefile.tmp
echo CPP=cl -E >>Makefile.tmp
echo CFLAGS=%CFLAGS% >>Makefile.tmp
echo CC=cl >>Makefile.tmp
echo YACC=bison -d -y >>Makefile.tmp
echo RANLIB=echo >>Makefile.tmp
echo O=obj >>Makefile.tmp
echo A=lib >>Makefile.tmp
echo EDIT_SOURCE=.\edit_source >>Makefile.tmp
echo EXE=.exe >>Makefile.tmp
echo TAB_EXT=_tab >> Makefile.tmp

echo ***************** Configuration completed **************
echo Installing MudOS on %ARCH%
echo.
echo Using copy to install binaries in %INSTALL_DIR%.
echo Using cl -E for preprocessing.
echo Using cl %CFLAGS% %OPTIMIZE% to compile.
echo Using bison -d -y to make the compiler.
echo Edit Makefile if this is not what you want
echo.
echo Otherwise, type 'nmake' to build MudOS, then 'nmake install'.

type Makefile.tmp > Makefile
type NMakefile.in >> Makefile
del Makefile.tmp
echo wsock32.lib > system_libs

set CFLAGS=
set OBJDIR=
set DRIVER_BIN=
set INSTALL_DIR=
set OPTIMIZE=
set DEBUG=
set ARCH=
