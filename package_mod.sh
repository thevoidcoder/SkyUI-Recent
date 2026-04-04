#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# package_mod.sh
# Linux helper: packages all non-C++ mod assets into an MO2-installable zip.
#
# Use this when you already have a pre-built SkyUIRecentSort.dll (compiled on
# Windows with build_mod.ps1) and just want to repackage the assets.
#
# Usage:
#   ./package_mod.sh [path/to/SkyUIRecentSort.dll] [path/to/SUIR_AcquiredBridge.pex]
#
#   Both arguments are optional. If omitted, placeholders are skipped and a
#   warning is printed. Drop the real files into dist/ before installing.
# ---------------------------------------------------------------------------
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MOD_NAME="SkyUIRecentSort"
VERSION="0.1.2"
DIST_DIR="$PROJECT_ROOT/dist"
STAGING_DIR="$DIST_DIR/staging"
ZIP_PATH="$DIST_DIR/${MOD_NAME}-v${VERSION}.zip"

DLL_SRC="${1:-}"
PEX_SRC="${2:-}"

# ---------------------------------------------------------------------------
step() { echo; echo "==> $*"; }
ok()   { echo "    [OK] $*"; }
warn() { echo "    [WARN] $*" >&2; }
fail() { echo "    [FAIL] $*" >&2; exit 1; }
# ---------------------------------------------------------------------------

command -v zip >/dev/null 2>&1 || fail "'zip' not found. Install it: sudo pacman -S zip"

step "Staging mod files"
rm -rf "$STAGING_DIR"

# SKSE/Plugins/SkyUIRecentSort.dll
mkdir -p "$STAGING_DIR/SKSE/Plugins"
if [[ -n "$DLL_SRC" && -f "$DLL_SRC" ]]; then
    cp "$DLL_SRC" "$STAGING_DIR/SKSE/Plugins/${MOD_NAME}.dll"
    ok "Staged: SKSE/Plugins/${MOD_NAME}.dll"
else
    warn "DLL not provided. Drop ${MOD_NAME}.dll into SKSE/Plugins/ inside the zip before installing."
fi

# Interface/skyui/config.txt - NO LONGER SHIPPED (injected via DLL)
# mkdir -p "$STAGING_DIR/Interface/skyui"
# cp "$PROJECT_ROOT/interface/skyui/config.txt" "$STAGING_DIR/Interface/skyui/config.txt"
# ok "Staged: Interface/skyui/config.txt"

# Scripts/Source/SUIR_AcquiredBridge.psc
mkdir -p "$STAGING_DIR/Scripts/Source"
cp "$PROJECT_ROOT/papyrus/Scripts/Source/SUIR_AcquiredBridge.psc" \
   "$STAGING_DIR/Scripts/Source/SUIR_AcquiredBridge.psc"
ok "Staged: Scripts/Source/SUIR_AcquiredBridge.psc"

# Scripts/SUIR_AcquiredBridge.pex  (compiled Papyrus — must be compiled on Windows)
mkdir -p "$STAGING_DIR/Scripts"
if [[ -n "$PEX_SRC" && -f "$PEX_SRC" ]]; then
    cp "$PEX_SRC" "$STAGING_DIR/Scripts/SUIR_AcquiredBridge.pex"
    ok "Staged: Scripts/SUIR_AcquiredBridge.pex"
else
    warn ".pex not provided. Compile SUIR_AcquiredBridge.psc with the Skyrim Papyrus Compiler on Windows."
    warn "Drop SUIR_AcquiredBridge.pex into Scripts/ inside the zip before installing."
fi

# ---------------------------------------------------------------------------
step "Creating $ZIP_PATH"
mkdir -p "$DIST_DIR"
rm -f "$ZIP_PATH"

(cd "$STAGING_DIR" && zip -r "$ZIP_PATH" .)
rm -rf "$STAGING_DIR"

echo
echo "============================================================"
echo "  Mod packaged: $ZIP_PATH"
echo "  Install in MO2: drag-and-drop the zip onto the MO2 window."
echo "  Load order: place AFTER SkyUI SE."
echo "============================================================"
