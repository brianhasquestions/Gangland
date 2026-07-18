@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSDIR="
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
)
if not defined VSDIR (
    for %%e in (Community Professional Enterprise BuildTools) do (
        if not defined VSDIR if exist "%ProgramFiles%\Microsoft Visual Studio\18\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VSDIR=%ProgramFiles%\Microsoft Visual Studio\18\%%e"
        if not defined VSDIR if exist "%ProgramFiles%\Microsoft Visual Studio\2022\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VSDIR=%ProgramFiles%\Microsoft Visual Studio\2022\%%e"
    )
)
if not defined VSDIR (
    echo Could not find a Visual Studio installation with C++ build tools.
    echo Install Visual Studio with the "Desktop development with C++" workload.
    exit /b 1
)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if not exist bin mkdir bin
rc /nologo /fo bin\gangland.res res\gangland.rc
if errorlevel 1 (
    echo RESOURCE COMPILE FAILED
    exit /b 1
)
cl /nologo /W4 /O1 /Os /GS- /D UNICODE /D _UNICODE /Fo:bin\ /Fe:bin\gangland.exe src\entry.c src\util.c src\main.c src\world.c src\menu.c src\actions.c src\combat.c src\sound.c bin\gangland.res /link /NODEFAULTLIB /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup kernel32.lib user32.lib gdi32.lib shlwapi.lib dwmapi.lib uxtheme.lib winmm.lib
if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)
echo BUILD OK: bin\gangland.exe
endlocal
