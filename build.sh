#!/bin/bash
set -e

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# ANSI Color Codes
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Helper to Add pg_config to PATH if not present
if ! command -v pg_config &> /dev/null; then
    # Try finding it in common relative build directories
    POSSIBLE_LOCATIONS=(
        "../../pg_carbon_build_dir/bin"
        "../../pg_carbon_installed/bin"
    )
    
    for loc in "${POSSIBLE_LOCATIONS[@]}"; do
        if [ -x "$loc/pg_config" ]; then
            ABS_LOC="$(cd "$loc" && pwd)"
            export PATH="$ABS_LOC:$PATH"
            echo "Found pg_config at: $ABS_LOC/pg_config"
            break
        fi
    done
fi

# Determine absolute install dir using pg_config
if ! command -v pg_config &> /dev/null; then
    echo -e "${RED}Error: pg_config not found in PATH.${NC}"
    echo -e "${RED}Please ensure postgres is installed and pg_config is available.${NC}"
    exit 1
fi

PG_BIN_DIR="$(pg_config --bindir)"
PG_INSTALL_DIR_ABS="$(cd "$PG_BIN_DIR/.." && pwd)"

echo "Using pg_config: $(which pg_config)"
echo -e "Target Install Dir: ${GREEN}${PG_INSTALL_DIR_ABS}${NC}"

BUILD_DIR="build_standalone"

# Configure Meson
if [ ! -d "$BUILD_DIR" ]; then
    echo "Configuring build in $BUILD_DIR..."
    meson setup "$BUILD_DIR"
else
    echo "Reconfiguring build in $BUILD_DIR..."
    meson setup "$BUILD_DIR" --reconfigure
fi

# Compile
echo "Compiling..."
meson compile -C "$BUILD_DIR"

# Install
echo "Installing..."
meson install -C "$BUILD_DIR"

echo "pg_carbon built and installed ${GREEN}successfully!${NC}"
