@echo off
setlocal enabledelayedexpansion

:: Check registry (MSYS2 installer registers its location here)
for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\MSYS2 64bit" /v InstallLocation 2^>nul ^| findstr "InstallLocation"') do set MSYS2_ROOT=%%b

:: Fall back to common install paths
if not defined MSYS2_ROOT (
    for %%d in (C:\msys64 C:\msys2 D:\msys64 E:\msys64 "%USERPROFILE%\msys64" "%LOCALAPPDATA%\msys64") do (
        if not defined MSYS2_ROOT (
            if exist "%%~d\mingw64\bin\gcc.exe" set MSYS2_ROOT=%%~d
        )
    )
)

if not defined MSYS2_ROOT (
    echo ERROR: MSYS2 not found.
    echo Install MSYS2 or set MSYS2_ROOT environment variable before running.
    pause
    exit /b 1
)

@echo Using MSYS2: %MSYS2_ROOT%
set PATH=%MSYS2_ROOT%\mingw64\bin;%MSYS2_ROOT%\usr\bin;%PATH%
:: Export MSYS2_ROOT so the editor inherits it for terminal profile in settings.json
setx MSYS2_ROOT "%MSYS2_ROOT%" >nul 2>&1

echo ===================================================
echo Select the editor you want to launch with MSYS2 paths:
echo [1] Antigravity IDE
echo [2] VS Code
echo ===================================================
choice /c 12 /n /m "Enter choice [1-2]: "
set CHOICE=%ERRORLEVEL%

if "%CHOICE%"=="1" (
    echo Launching Antigravity IDE...
    :: Find Antigravity.exe or Antigravity IDE.exe directly
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
        start "" "!ANTIGRAVITY_EXE!" "%~dp0."
    ) else (
        where antigravity-ide >nul 2>&1
        if !ERRORLEVEL! equ 0 (
            start /b "" cmd /c antigravity-ide "%~dp0."
        ) else (
            where antigravity >nul 2>&1
            if !ERRORLEVEL! equ 0 (
                start /b "" cmd /c antigravity "%~dp0."
            ) else (
                echo ERROR: Antigravity executable not found.
                pause
                exit /b 1
            )
        )
    )
)

if "%CHOICE%"=="2" (
    echo Launching VS Code...
    set "CODE_EXE="
    for %%p in (
        "%LOCALAPPDATA%\Programs\Microsoft VS Code\Code.exe"
        "C:\Program Files\Microsoft VS Code\Code.exe"
        "C:\Program Files (x86)\Microsoft VS Code\Code.exe"
    ) do if not defined CODE_EXE if exist "%%~p" set "CODE_EXE=%%~p"
    
    if defined CODE_EXE (
        start "" "!CODE_EXE!" "%~dp0."
    ) else (
        start /b "" cmd /c code "%~dp0."
    )
)
