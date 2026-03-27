# IEB Menu Build System (PowerShell Edition)
$ErrorActionPreference = "Continue"

Write-Host "####################################" -ForegroundColor Cyan
Write-Host "#      IEB Menu Build System       #" -ForegroundColor Cyan
Write-Host "####################################" -ForegroundColor Cyan

# 1. Check for CMake
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "[!] CMake bulunamadi. Winget ile kuruluyor..." -ForegroundColor Yellow
    winget install kitware.cmake --silent --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[!] CMake kurulumu basarisiz! Lutfen manuel kurun: https://cmake.org/" -ForegroundColor Red
        Read-Host "Devam etmek icin Enter'a basin..."
        exit
    }
    $env:Path += ";C:\Program Files\CMake\bin"
}

# 1.5 Check for Ninja
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    Write-Host "[!] Ninja (Build Generator) bulunamadi. Winget ile kuruluyor..." -ForegroundColor Yellow
    winget install Ninja-build.Ninja --silent --accept-source-agreements --accept-package-agreements
}

# 2. Check for Geode CLI
if (-not (Get-Command geode -ErrorAction SilentlyContinue)) {
    Write-Host "[!] Geode CLI bulunamadi. Winget ile kuruluyor..." -ForegroundColor Yellow
    winget install GeodeSDK.GeodeCLI --silent --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[!] Geode CLI kurulumu basarisiz! Lutfen manuel kurun." -ForegroundColor Red
        Read-Host "Devam etmek icin Enter'a basin..."
        exit
    }
    $env:Path += ";$env:USERPROFILE\.geode\bin"
}

# 3. Geode Profile Setup
$profiles = geode profile list 2>$null
if (-not $profiles) {
    Write-Host "[!] Geode profili bulunamadi. Otomatik kurulum deneniyor..." -ForegroundColor Yellow
    $gdPath = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
    if (Test-Path $gdPath) {
        Write-Host "[*] Varsayilan profil ekleniyor: $gdPath" -ForegroundColor Green
        geode profile add --name default "$gdPath" win
    } else {
        Write-Host "[!] Geometry Dash klasoru bulunamadi. Lutfen 'geode config setup' komutunu calistirin." -ForegroundColor Red
        Read-Host "Devam etmek icin Enter'a basin..."
        exit
    }
}

# 4. Geode SDK Detection
$sdkPath = geode sdk path 2>$null
if (-not $sdkPath) {
    if (Test-Path "$env:USERPROFILE\Documents\Geode") {
        $sdkPath = "$env:USERPROFILE\Documents\Geode"
    }
}

if (-not $sdkPath) {
    Write-Host "[!] Geode SDK bulunamadi. Kuruluyor..." -ForegroundColor Yellow
    geode sdk install
    geode sdk install-binaries
    $sdkPath = geode sdk path 2>$null
}

if (-not $sdkPath) {
    Write-Host "[!] SDK yolu hala bulunamadi. Lutfen bu pencereyi kapatip tekrar acin." -ForegroundColor Red
    Read-Host "Devam etmek icin Enter'a basin..."
    exit
}

Write-Host "[*] SDK Yolu: $sdkPath" -ForegroundColor Green
Write-Host "[*] Ortam hazir. Mod derleniyor..." -ForegroundColor Cyan

# 4.5 Force Ninja and MSVC (New)
$env:CMAKE_GENERATOR = "Ninja"

# Try to find vcvars64.bat to initialize environment if cl.exe is missing
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vsPath = Get-ChildItem -Path 'C:\Program Files\Microsoft Visual Studio\*\Community\VC\Auxiliary\Build\vcvars64.bat' -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
    if ($vsPath -and (Test-Path $vsPath)) {
        Write-Host "[*] Visual Studio ortami hazirlaniyor: $vsPath" -ForegroundColor Gray
        $envVars = cmd /c "`"$vsPath`" x64 && set"
        foreach ($line in $envVars) {
            if ($line -match "^(.*?)=(.*)$") {
                $key = $Matches[1]
                $val = $Matches[2]
                if ($key -ne "PSModulePath") {
                    Set-Item -Path "Env:$key" -Value $val
                }
            }
        }
    }
}

# 5. Build
if (Test-Path "build") {
    Write-Host "[*] Eski derleme dosyalari temizleniyor..." -ForegroundColor Gray
    Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
}

# Run geode build
    # Clean and build with VS 2022
    if (Test-Path "build") { Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue }
    New-Item -ItemType Directory -Path "build" | Out-Null
    
    Write-Host "[*] Visual Studio 2022 ile derleme basliyor..." -ForegroundColor Cyan
    cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo
    if ($LASTEXITCODE -eq 0) {
        cmake --build build --config RelWithDebInfo
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[*] Paket olusturuluyor..." -ForegroundColor Cyan
            geode package build build
        }
    }

    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n[!] Derleme sirasinda bir hata olustu!" -ForegroundColor Red
        Write-Host "[?] Not: Visual Studio 2022 (C++ ile masaustu gelistirme) kurulu oldugundan emin olun." -ForegroundColor Yellow
        Read-Host "Devam etmek icin Enter'a basin..."
        exit
    }

Write-Host "`n[V] Derleme BASARILI!" -ForegroundColor Green
Write-Host "[*] Derlenen dosya: build\ieb.ieb_menu.geode" -ForegroundColor Green

# 6. Install to GD
$install = Read-Host "`nOyuna otomatik yuklemek istiyor musunuz? (Y/N)"
if ($install -eq "Y" -or $install -eq "y") {
    $gdPath = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
    if (Test-Path $gdPath) {
        if (-not (Test-Path "$gdPath\geode\mods")) {
            New-Item -ItemType Directory -Force -Path "$gdPath\geode\mods" | Out-Null
        }
        Copy-Item "build\ieb.ieb_menu.geode" -Destination "$gdPath\geode\mods\" -Force
        Write-Host "[V] Mod oyun klasorune kopyalandi!" -ForegroundColor Green
    }
}

Read-Host "`nIslem tamamlandi. Cikmak icin Enter'a basin..."
