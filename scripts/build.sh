#!/bin/bash
#
# Build script for FreeRTOS STM32F4 examples
#

set -e

# ------------------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
EXAMPLES_DIR="$PROJECT_ROOT/examples"
TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/arm-none-eabi.cmake"

BUILD_TYPE="Debug"
PARALLEL_JOBS=8

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ------------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------------

print_info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[OK]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
print_error()   { echo -e "${RED}[ERROR]${NC} $1"; }

list_examples() {
    echo "Available examples:"
    for dir in "$EXAMPLES_DIR"/*/; do
        if [[ -f "${dir}main.c" ]]; then
            echo "  - $(basename "$dir")"
        fi
    done
}

usage() {
    echo "Usage: $(basename "$0") [OPTIONS] <example_name>"
    echo ""
    echo "Build a FreeRTOS example for STM32F4."
    echo ""
    echo "Options:"
    echo "  -c, --clean       Clean build directory before building"
    echo "  -r, --release     Build in Release mode (default: Debug)"
    echo "  -j, --jobs N      Number of parallel jobs (default: $PARALLEL_JOBS)"
    echo "  -l, --list        List available examples"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $(basename "$0") basic_blink"
    echo "  $(basename "$0") -c -r creating_tasks"
    echo ""
    list_examples
}

# ------------------------------------------------------------------------------
# Parse arguments
# ------------------------------------------------------------------------------

CLEAN_BUILD=false
EXAMPLE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        -l|--list)
            list_examples
            exit 0
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            EXAMPLE="$1"
            shift
            ;;
    esac
done

# ------------------------------------------------------------------------------
# Validate
# ------------------------------------------------------------------------------

if [[ -z "$EXAMPLE" ]]; then
    print_error "No example specified"
    echo ""
    usage
    exit 1
fi

if [[ ! -d "$EXAMPLES_DIR/$EXAMPLE" ]]; then
    print_error "Example '$EXAMPLE' not found"
    echo ""
    list_examples
    exit 1
fi

if [[ ! -f "$EXAMPLES_DIR/$EXAMPLE/main.c" ]]; then
    print_error "No main.c found in '$EXAMPLE'"
    exit 1
fi

# ------------------------------------------------------------------------------
# Build
# ------------------------------------------------------------------------------

cd "$PROJECT_ROOT"

if [[ "$CLEAN_BUILD" == true ]]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

print_info "Configuring: $EXAMPLE ($BUILD_TYPE)"
cmake -S . -B build \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DEXAMPLE="$EXAMPLE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo ""
print_info "Building with $PARALLEL_JOBS jobs..."
cmake --build build -j "$PARALLEL_JOBS"

echo ""
print_success "Build complete: build/freertos-stm32f4.elf"
