#!/bin/bash
#
# Flash script for STM32F4 using ST-Link
#

set -e

# ------------------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_FILE="$BUILD_DIR/freertos-stm32f4-for-learning.bin"
ELF_FILE="$BUILD_DIR/freertos-stm32f4-for-learning.elf"
FLASH_ADDR="0x08000000"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# ------------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------------

print_info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[OK]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
print_error()   { echo -e "${RED}[ERROR]${NC} $1"; }

usage() {
    echo "Usage: $(basename "$0") [OPTIONS]"
    echo ""
    echo "Flash firmware to STM32F4 using ST-Link."
    echo ""
    echo "Options:"
    echo "  -e, --erase       Erase flash before programming"
    echo "  -v, --verify      Verify after flashing"
    echo "  -r, --reset       Reset after flashing (default)"
    echo "  -n, --no-reset    Don't reset after flashing"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $(basename "$0")"
    echo "  $(basename "$0") --erase --verify"
}

check_dependencies() {
    if ! command -v st-flash &> /dev/null; then
        print_error "st-flash not found. Install stlink tools:"
        echo "  macOS:  brew install stlink"
        echo "  Ubuntu: apt install stlink-tools"
        exit 1
    fi
}

# ------------------------------------------------------------------------------
# Parse arguments
# ------------------------------------------------------------------------------

DO_ERASE=false
DO_VERIFY=false
DO_RESET=true

while [[ $# -gt 0 ]]; do
    case $1 in
        -e|--erase)
            DO_ERASE=true
            shift
            ;;
        -v|--verify)
            DO_VERIFY=true
            shift
            ;;
        -r|--reset)
            DO_RESET=true
            shift
            ;;
        -n|--no-reset)
            DO_RESET=false
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# ------------------------------------------------------------------------------
# Validate
# ------------------------------------------------------------------------------

check_dependencies

if [[ ! -f "$BIN_FILE" ]]; then
    print_error "Binary not found: $BIN_FILE"
    echo "Run './scripts/build.sh <example>' first."
    exit 1
fi

# ------------------------------------------------------------------------------
# Flash
# ------------------------------------------------------------------------------

cd "$PROJECT_ROOT"

# Show binary info
BIN_SIZE=$(stat -f%z "$BIN_FILE" 2>/dev/null || stat -c%s "$BIN_FILE" 2>/dev/null)
print_info "Binary: $BIN_FILE ($BIN_SIZE bytes)"

# Erase if requested
if [[ "$DO_ERASE" == true ]]; then
    print_info "Erasing flash..."
    st-flash erase
fi

# Build st-flash command
FLASH_CMD="st-flash"
if [[ "$DO_RESET" == true ]]; then
    FLASH_CMD="$FLASH_CMD --reset"
fi
FLASH_CMD="$FLASH_CMD write $BIN_FILE $FLASH_ADDR"

# Flash
print_info "Flashing to $FLASH_ADDR..."
eval "$FLASH_CMD"

# Verify if requested
if [[ "$DO_VERIFY" == true ]]; then
    print_info "Verifying..."
    st-flash read /tmp/verify.bin $FLASH_ADDR "$BIN_SIZE"
    if cmp -s "$BIN_FILE" /tmp/verify.bin; then
        print_success "Verification passed"
    else
        print_error "Verification failed!"
        rm -f /tmp/verify.bin
        exit 1
    fi
    rm -f /tmp/verify.bin
fi

print_success "Flash complete"
