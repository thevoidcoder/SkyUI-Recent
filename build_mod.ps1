#Requires -Version 7.0
<#
.SYNOPSIS
    Builds SkyUIRecentSort and packages it as an MO2-installable zip.

.DESCRIPTION
    1. Compiles the SKSE plugin (C++ DLL) via CMake + vcpkg.
    2. Optionally compiles the Papyrus script if Skyrim is installed.
    3. Packages everything into a zip with the correct Data/ layout for MO2.

.PARAMETER SkyrimPath
    Path to your Skyrim Special Edition install folder.
    Defaults to the standard Steam location.

.PARAMETER VcpkgRoot
    Path to your vcpkg installation.
    Defaults to the VCPKG_ROOT environment variable.

.PARAMETER SkipPapyrus
    Skip Papyrus compilation (use if Skyrim is not installed on this machine).

.PARAMETER Configuration
    CMake build configuration. Default: Release.

.EXAMPLE
    .\build_mod.ps1
    .\build_mod.ps1 -SkyrimPath "D:\Games\Skyrim Special Edition" -VcpkgRoot "C:\vcpkg"
    .\build_mod.ps1 -SkipPapyrus
#>

param(
    [string] $SkyrimPath     = "$env:ProgramFiles(x86)\Steam\steamapps\common\Skyrim Special Edition",
    [string] $VcpkgRoot      = $env:VCPKG_ROOT,
    [switch] $SkipPapyrus    = $false,
    [string] $Configuration  = "Release"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
$ProjectRoot = $PSScriptRoot
$PluginSrc   = Join-Path $ProjectRoot "skse_plugin"
$BuildDir    = Join-Path $PluginSrc   "build"
$DistDir     = Join-Path $ProjectRoot "dist"
$ModName     = "SkyUIRecentSort"
$Version     = "0.1.0"
$ZipName     = "$ModName-v$Version.zip"
$ZipPath     = Join-Path $DistDir $ZipName
$StagingDir  = Join-Path $DistDir "staging"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
function Step([string]$msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }
function Ok([string]$msg)   { Write-Host "    [OK] $msg" -ForegroundColor Green }
function Warn([string]$msg) { Write-Host "    [WARN] $msg" -ForegroundColor Yellow }
function Fail([string]$msg) { Write-Host "    [FAIL] $msg" -ForegroundColor Red; exit 1 }

# ---------------------------------------------------------------------------
# Step 1 — Validate prerequisites
# ---------------------------------------------------------------------------
Step "Checking prerequisites"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Fail "cmake not found. Install CMake and add it to PATH."
}
Ok "cmake found: $(cmake --version | Select-Object -First 1)"

if ([string]::IsNullOrEmpty($VcpkgRoot) -or -not (Test-Path $VcpkgRoot)) {
    Fail "vcpkg not found. Set VCPKG_ROOT or pass -VcpkgRoot. Get vcpkg from https://github.com/microsoft/vcpkg"
}
$VcpkgToolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $VcpkgToolchain)) {
    Fail "vcpkg toolchain file not found at: $VcpkgToolchain"
}
Ok "vcpkg found: $VcpkgRoot"

# ---------------------------------------------------------------------------
# Step 2 — Configure with CMake
# ---------------------------------------------------------------------------
Step "Configuring CMake (vcpkg manifest mode)"

$cmakeArgs = @(
    "-B", $BuildDir,
    "-S", $PluginSrc,
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain",
    "-DVCPKG_TARGET_TRIPLET=x64-windows-static",
    "-DCMAKE_BUILD_TYPE=$Configuration"
)

cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { Fail "CMake configure failed." }
Ok "CMake configured."

# ---------------------------------------------------------------------------
# Step 3 — Build the DLL
# ---------------------------------------------------------------------------
Step "Building $ModName.dll ($Configuration)"

cmake --build $BuildDir --config $Configuration
if ($LASTEXITCODE -ne 0) { Fail "CMake build failed." }

$DllPath = Get-ChildItem -Path $BuildDir -Filter "$ModName.dll" -Recurse |
    Where-Object { $_.FullName -match "\\$Configuration\\" } |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $DllPath) {
    # Fallback: any location
    $DllPath = Get-ChildItem -Path $BuildDir -Filter "$ModName.dll" -Recurse |
        Select-Object -First 1 -ExpandProperty FullName
}
if (-not $DllPath) { Fail "Could not locate built $ModName.dll under $BuildDir" }
Ok "DLL: $DllPath"

# ---------------------------------------------------------------------------
# Step 4 — Compile Papyrus (optional)
# ---------------------------------------------------------------------------
$PexPath = $null

if (-not $SkipPapyrus) {
    Step "Compiling Papyrus scripts"

    $PapyrusCompiler = Join-Path $SkyrimPath "Papyrus Compiler\PapyrusCompiler.exe"
    $FlagsFile       = Join-Path $SkyrimPath "Papyrus Compiler\TESV_Papyrus_Flags.flg"
    $SkyrimScripts   = Join-Path $SkyrimPath "Data\Scripts\Source"
    $PscDir          = Join-Path $ProjectRoot "papyrus\Scripts\Source"
    $PexOutDir       = Join-Path $DistDir "pex_temp"

    if (-not (Test-Path $PapyrusCompiler)) {
        Warn "Papyrus compiler not found at: $PapyrusCompiler"
        Warn "Skipping Papyrus compilation. Add the .pex manually or re-run with -SkipPapyrus."
    } else {
        New-Item -ItemType Directory -Path $PexOutDir -Force | Out-Null

        & $PapyrusCompiler "$PscDir\SUIR_AcquiredBridge.psc" `
            -f="$FlagsFile" `
            -i="$SkyrimScripts;$PscDir" `
            -o="$PexOutDir" `
            -op

        if ($LASTEXITCODE -ne 0) { Fail "Papyrus compilation failed." }

        $PexPath = Join-Path $PexOutDir "SUIR_AcquiredBridge.pex"
        if (-not (Test-Path $PexPath)) { Fail "Expected .pex not found: $PexPath" }
        Ok "Papyrus compiled: $PexPath"
    }
} else {
    Warn "Papyrus compilation skipped (-SkipPapyrus). Distribute a pre-compiled .pex if needed."
}

# ---------------------------------------------------------------------------
# Step 5 — Stage files with MO2-compatible Data/ layout
# ---------------------------------------------------------------------------
Step "Staging mod files"

if (Test-Path $StagingDir) { Remove-Item $StagingDir -Recurse -Force }
New-Item -ItemType Directory -Path $StagingDir | Out-Null

# SKSE/Plugins/SkyUIRecentSort.dll
$SksePluginsDir = Join-Path $StagingDir "SKSE\Plugins"
New-Item -ItemType Directory -Path $SksePluginsDir -Force | Out-Null
Copy-Item $DllPath (Join-Path $SksePluginsDir "$ModName.dll")
Ok "Staged: SKSE\Plugins\$ModName.dll"

# Interface/skyui/config.txt
$InterfaceDir = Join-Path $StagingDir "Interface\skyui"
New-Item -ItemType Directory -Path $InterfaceDir -Force | Out-Null
Copy-Item (Join-Path $ProjectRoot "interface\skyui\config.txt") (Join-Path $InterfaceDir "config.txt")
Ok "Staged: Interface\skyui\config.txt"

# Scripts\Source\SUIR_AcquiredBridge.psc
$ScriptSrcDir = Join-Path $StagingDir "Scripts\Source"
New-Item -ItemType Directory -Path $ScriptSrcDir -Force | Out-Null
Copy-Item (Join-Path $ProjectRoot "papyrus\Scripts\Source\SUIR_AcquiredBridge.psc") `
          (Join-Path $ScriptSrcDir "SUIR_AcquiredBridge.psc")
Ok "Staged: Scripts\Source\SUIR_AcquiredBridge.psc"

# Scripts\SUIR_AcquiredBridge.pex  (compiled Papyrus, if available)
if ($PexPath -and (Test-Path $PexPath)) {
    $ScriptsDir = Join-Path $StagingDir "Scripts"
    New-Item -ItemType Directory -Path $ScriptsDir -Force | Out-Null
    Copy-Item $PexPath (Join-Path $ScriptsDir "SUIR_AcquiredBridge.pex")
    Ok "Staged: Scripts\SUIR_AcquiredBridge.pex"
}

# ---------------------------------------------------------------------------
# Step 6 — Zip
# ---------------------------------------------------------------------------
Step "Creating zip: $ZipName"

New-Item -ItemType Directory -Path $DistDir -Force | Out-Null
if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }

Compress-Archive -Path "$StagingDir\*" -DestinationPath $ZipPath
Ok "Created: $ZipPath"

# Cleanup staging
Remove-Item $StagingDir -Recurse -Force

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "============================================================" -ForegroundColor Green
Write-Host "  Mod packaged: $ZipPath" -ForegroundColor Green
Write-Host "  Install in MO2: drag-and-drop the zip onto the MO2 window," -ForegroundColor Green
Write-Host "  or use: Mods -> Install Mod -> select the zip."             -ForegroundColor Green
Write-Host "  Load order: place AFTER SkyUI SE."                          -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
