@echo off
set "GEODE_SDK=V:"
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\Program Files\Git\bin;C:\Program Files\Git\cmd;%PATH%"

echo [*] Activating Visual Studio Environment...
call "%VS_PATH%" x64
if %errorlevel% neq 0 (
    echo [!] Failed to activate VS environment.
    exit /b 1
)

set "SOURCE_DIR=%~dp0"
set "TEMP_DIR=C:\Users\Public\ieb_build_final"

echo [*] Preparing Temp Build Directory: %TEMP_DIR%
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"
mkdir "%TEMP_DIR%"

echo [*] Copying source to temp directory...
robocopy "%SOURCE_DIR%." "%TEMP_DIR%" /E /XD build .git /NFL /NDL /NJH /NJS /nc /ns /np

echo [*] Switching to temp directory...
cd /D "%TEMP_DIR%"

echo [*] Configuring Project (Ninja)...
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo [!] CMake configuration failed.
    exit /b %errorlevel%
)

echo [*] Compiling Project (Ninja Build)...
ninja -C build
if %errorlevel% neq 0 (
    echo [!] Compilation failed.
    exit /b %errorlevel%
)

echo [*] Installation Complete (Handled by CMake post-build).

set "GD_MODS=C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash\geode\mods"
echo [*] Installing to %GD_MODS%...
if not exist "%GD_MODS%" mkdir "%GD_MODS%"
copy build\ieb.ieb_menu.geode "%GD_MODS%\" /y
if %errorlevel% neq 0 (
    echo [!] Installation failed.
    exit /b %errorlevel%
)

echo [V] IEB Menu v1.0.1 Build and Install SUCCESSFUL!
