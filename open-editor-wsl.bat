@echo off
setlocal enabledelayedexpansion

:: ===================================================================
:: open-editor-wsl.bat — Launch Antigravity IDE or VS Code under WSL
::
:: Usage:
::  1. Set up a WSL distribution (Ubuntu or Alpine) and clone this
::     repository into the native Linux filesystem:
::       wsl -d Ubuntu
::       git clone https://github.com/fluffos/fluffos ~/fluffos
::  2. In Windows Explorer, navigate to the project via its WSL UNC path:
::       \\wsl.localhost\Ubuntu\home\<user>\fluffos
::  3. Double-click this script from that UNC path.
::
:: The script reads the WSL distro and Linux path directly from the UNC
:: path, then launches your chosen editor with --remote wsl+<distro>.
:: ===================================================================

set "CURR_PATH=%~dp0"
:: Remove trailing slash
if "%CURR_PATH:~-1%"=="\" set "CURR_PATH=%CURR_PATH:~0,-1%"

:: ------------------------------------------------------------------
:: Step 1: Verify the script is launched from a WSL UNC path
:: ------------------------------------------------------------------
set "IS_WSL=0"
echo !CURR_PATH! | findstr /I "\\\\wsl.localhost\\" >nul && set "IS_WSL=1"
echo !CURR_PATH! | findstr /I "\\\\wsl$\\" >nul && set "IS_WSL=2"

if "%IS_WSL%"=="0" (
    echo ======================================================================
    echo  WARNING: This script was launched from a Windows drive.
    echo  Path: !CURR_PATH!
    echo.
    echo  For optimal performance, the codebase MUST reside in the native
    echo  Linux volume of your WSL distribution, NOT on a mapped Windows
    echo  drive ^(/mnt/c/...^). Cross-filesystem I/O is extremely slow.
    echo ======================================================================
    echo.
    echo  To fix this:
    echo   1. Open your WSL terminal: wsl -d Ubuntu
    echo   2. Clone/move the repo into the Linux home directory:
    echo        git clone https://github.com/fluffos/fluffos ~/fluffos
    echo   3. Open the folder in Windows Explorer via:
    echo        \\wsl.localhost\Ubuntu\home\^<user^>\fluffos
    echo   4. Run this batch file from there.
    echo.
    pause
    exit /b 1
)

:: ------------------------------------------------------------------
:: Step 2: Extract distro and Linux path from the UNC path
:: ------------------------------------------------------------------
:: CURR_PATH examples:
::   \\wsl.localhost\Ubuntu\home\user\fluffos
::   \\wsl$\Alpine\home\user\fluffos
if "%IS_WSL%"=="1" (
    set "TEMP_PATH=!CURR_PATH:\\wsl.localhost\=!"
) else (
    set "TEMP_PATH=!CURR_PATH:\\wsl$\=!"
)

:: TEMP_PATH is now: Ubuntu\home\user\fluffos
:: Split into distro name and the rest of the linux path
for /f "tokens=1* delims=\" %%a in ("!TEMP_PATH!") do (
    set "SELECTED_DISTRO=%%a"
    set "LINUX_PATH=%%b"
)

:: Convert backslashes to forward slashes
set "LINUX_PATH=/!LINUX_PATH:\=/!"

echo  WSL distro : !SELECTED_DISTRO!
echo  Linux path : !LINUX_PATH!
echo.

:: ------------------------------------------------------------------
:: Step 3: Choose the editor
:: ------------------------------------------------------------------
echo ===================================================
echo  Select the editor to launch:
echo  [1] Antigravity IDE
echo  [2] VS Code
echo ===================================================
choice /c 12 /n /m "Enter choice [1-2]: "
set EDITOR_CHOICE=%ERRORLEVEL%

:: ------------------------------------------------------------------
:: Step 4: Launch the editor with --remote wsl+<distro>
:: ------------------------------------------------------------------
if "%EDITOR_CHOICE%"=="1" (
    echo.
    echo Launching Antigravity IDE ^(WSL: !SELECTED_DISTRO!^)...

    set "ANTIGRAVITY_EXE="
    for %%p in (
        "%LOCALAPPDATA%\Programs\Antigravity IDE\Antigravity IDE.exe"
        "%LOCALAPPDATA%\Programs\Antigravity\Antigravity.exe"
        "C:\Program Files\Antigravity IDE\Antigravity IDE.exe"
        "C:\Program Files\Antigravity\Antigravity.exe"
        "C:\Program Files (x86)\Antigravity IDE\Antigravity IDE.exe"
        "C:\Program Files (x86)\Antigravity\Antigravity.exe"
    ) do if not defined ANTIGRAVITY_EXE if exist "%%~p" set "ANTIGRAVITY_EXE=%%~p"

    if defined ANTIGRAVITY_EXE (
        start "" "!ANTIGRAVITY_EXE!" --remote wsl+!SELECTED_DISTRO! "!LINUX_PATH!"
    ) else (
        where antigravity-ide >nul 2>&1
        if !ERRORLEVEL! equ 0 (
            start /b "" cmd /c antigravity-ide --remote wsl+!SELECTED_DISTRO! "!LINUX_PATH!"
        ) else (
            where antigravity >nul 2>&1
            if !ERRORLEVEL! equ 0 (
                start /b "" cmd /c antigravity --remote wsl+!SELECTED_DISTRO! "!LINUX_PATH!"
            ) else (
                echo ERROR: Antigravity executable not found.
                pause
                exit /b 1
            )
        )
    )
)

if "%EDITOR_CHOICE%"=="2" (
    echo.
    echo Launching VS Code ^(WSL: !SELECTED_DISTRO!^)...

    set "CODE_EXE="
    for %%p in (
        "%LOCALAPPDATA%\Programs\Microsoft VS Code\Code.exe"
        "C:\Program Files\Microsoft VS Code\Code.exe"
        "C:\Program Files (x86)\Microsoft VS Code\Code.exe"
    ) do if not defined CODE_EXE if exist "%%~p" set "CODE_EXE=%%~p"

    if defined CODE_EXE (
        start "" "!CODE_EXE!" --remote wsl+!SELECTED_DISTRO! "!LINUX_PATH!"
    ) else (
        start /b "" cmd /c code --remote wsl+!SELECTED_DISTRO! "!LINUX_PATH!"
    )
)

echo Done.
endlocal
