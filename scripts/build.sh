#!/bin/bash
# QtForge Unix/Linux/macOS/MSYS2 Build Script
# Automates building and packaging for Unix-like systems and MSYS2

set -e  # Exit on any error

# Default configuration
BUILD_TYPE="Release"
BUILD_DIR="$(dirname "$0")/../build"
INSTALL_DIR="$(dirname "$0")/../install"
SOURCE_DIR="$(dirname "$0")/.."
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
BUILD_TESTS="ON"
BUILD_EXAMPLES="ON"
BUILD_NETWORK="ON"
BUILD_UI="ON"
CREATE_PACKAGE="OFF"
CLEAN_BUILD="OFF"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show help
show_help() {
    cat << EOF
QtPlugin Unix/Linux/macOS Build Script

Usage: $0 [options]

Options:
  --help, -h      Show this help message
  --debug         Build in Debug mode (default: Release)
  --release       Build in Release mode
  --tests         Build and run tests
  --network       Build network support
  --ui            Build UI support
  --package       Create packages after build
  --clean         Clean build directory before building
  --jobs N        Number of parallel jobs (default: auto-detect)

Examples:
  $0                           # Basic release build
  $0 --debug --tests           # Debug build with tests
  $0 --release --package       # Release build with packaging
  $0 --clean --network --ui    # Clean build with all features

EOF
}

# Function to detect system
detect_system() {
    # Check for MSYS2 environment first
    if [[ -n "$MSYSTEM" ]]; then
        SYSTEM="msys2"
        MSYS2_SUBSYSTEM="$MSYSTEM"
        PACKAGE_GENERATORS="ZIP;NSIS"
        print_status "Detected MSYS2 environment: $MSYSTEM"

        # Set toolchain based on MSYS2 subsystem
        case "$MSYSTEM" in
            MINGW64)
                CMAKE_TOOLCHAIN="cmake/toolchains/msys2-mingw64.cmake"
                ;;
            UCRT64)
                CMAKE_TOOLCHAIN="cmake/toolchains/msys2-ucrt64.cmake"
                ;;
            CLANG64|CLANG32)
                print_warning "MSYS2 Clang environment detected. Using default configuration."
                ;;
            MSYS)
                print_warning "MSYS2 MSYS environment detected. Consider using MINGW64 or UCRT64 for better compatibility."
                ;;
        esac
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        SYSTEM="linux"
        PACKAGE_GENERATORS="DEB;RPM;TGZ"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        SYSTEM="macos"
        PACKAGE_GENERATORS="DragNDrop;TGZ"
    elif [[ "$OSTYPE" == "freebsd"* ]]; then
        SYSTEM="freebsd"
        PACKAGE_GENERATORS="TGZ"
    else
        SYSTEM="unix"
        PACKAGE_GENERATORS="TGZ"
    fi

    print_status "Detected system: $SYSTEM"
}

# Function to detect Qt
detect_qt() {
    QT_FOUND=0
    QT_DIR=""

    # Check environment variables
    if [[ -n "$Qt6_DIR" && -d "$Qt6_DIR" ]]; then
        QT_FOUND=1
        QT_DIR="$Qt6_DIR"
        print_status "Found Qt6 from Qt6_DIR: $QT_DIR"
        return
    fi

    if [[ -n "$QT_DIR" && -d "$QT_DIR" ]]; then
        QT_FOUND=1
        print_status "Found Qt6 from QT_DIR: $QT_DIR"
        return
    fi

    # Check common paths based on system
    if [[ "$SYSTEM" == "macos" ]]; then
        QT_PATHS=(
            "/usr/local/Qt-6.6.0"
            "/usr/local/Qt-6.5.0"
            "/opt/homebrew/opt/qt@6"
            "/usr/local/opt/qt@6"
            "/Applications/Qt/6.6.0/macos"
            "/Applications/Qt/6.5.0/macos"
        )
    elif [[ "$SYSTEM" == "msys2" ]]; then
        # MSYS2 Qt paths
        if [[ -n "$MSYSTEM_PREFIX" ]]; then
            QT_PATHS=(
                "$MSYSTEM_PREFIX/lib/cmake/Qt6"
                "$MSYSTEM_PREFIX"
            )
        else
            QT_PATHS=(
                "/mingw64/lib/cmake/Qt6"
                "/ucrt64/lib/cmake/Qt6"
                "/clang64/lib/cmake/Qt6"
                "/mingw64"
                "/ucrt64"
                "/clang64"
            )
        fi
    else
        QT_PATHS=(
            "/usr/lib/qt6"
            "/usr/local/Qt-6.6.0"
            "/usr/local/Qt-6.5.0"
            "/opt/qt6"
            "/usr/lib/x86_64-linux-gnu/qt6"
        )
    fi

    for path in "${QT_PATHS[@]}"; do
        if [[ -d "$path" ]]; then
            QT_FOUND=1
            QT_DIR="$path"
            print_status "Found Qt6 at: $QT_DIR"
            return
        fi
    done

    # Try pkg-config
    if command -v pkg-config >/dev/null 2>&1; then
        if pkg-config --exists Qt6Core; then
            QT_FOUND=1
            print_status "Found Qt6 via pkg-config"
            return
        fi
    fi

    print_warning "Qt6 not found in standard locations, relying on system PATH"
}

# Function to detect build tools
detect_build_tools() {
    # Check for CMake
    if ! command -v cmake >/dev/null 2>&1; then
        print_error "CMake not found. Please install CMake 3.21 or later."
        exit 1
    fi

    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_status "Found CMake version: $CMAKE_VERSION"

    # Check for Ninja (preferred) or Make
    if command -v ninja >/dev/null 2>&1; then
        CMAKE_GENERATOR="-G Ninja"
        print_status "Using Ninja build system"
    elif command -v make >/dev/null 2>&1; then
        CMAKE_GENERATOR="-G \"Unix Makefiles\""
        print_status "Using Make build system"
    else
        print_warning "Neither Ninja nor Make found, using default generator"
        CMAKE_GENERATOR=""
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --tests)
            BUILD_TESTS="ON"
            shift
            ;;
        --network)
            BUILD_NETWORK="ON"
            shift
            ;;
        --ui)
            BUILD_UI="ON"
            shift
            ;;
        --package)
            CREATE_PACKAGE="ON"
            shift
            ;;
        --clean)
            CLEAN_BUILD="ON"
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Main build process
main() {
    echo "========================================"
    echo "QtPlugin Unix/Linux/macOS Build Script"
    echo "========================================"
    echo "Build Type: $BUILD_TYPE"
    echo "Build Directory: $BUILD_DIR"
    echo "Install Directory: $INSTALL_DIR"
    echo "Parallel Jobs: $PARALLEL_JOBS"
    echo "Build Tests: $BUILD_TESTS"
    echo "Build Examples: $BUILD_EXAMPLES"
    echo "Build Network: $BUILD_NETWORK"
    echo "Build UI: $BUILD_UI"
    echo "Create Package: $CREATE_PACKAGE"
    echo "========================================"
    echo

    # Detect system and tools
    detect_system
    detect_build_tools
    detect_qt

    # Clean build directory if requested
    if [[ "$CLEAN_BUILD" == "ON" ]]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi

    # Create directories
    mkdir -p "$BUILD_DIR"
    mkdir -p "$INSTALL_DIR"

    # Configure CMake arguments
    CMAKE_ARGS=(
        -S "$SOURCE_DIR"
        -B "$BUILD_DIR"
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
        -DQTFORGE_BUILD_TESTS="$BUILD_TESTS"
        -DQTFORGE_BUILD_EXAMPLES="$BUILD_EXAMPLES"
        -DQTFORGE_BUILD_NETWORK="$BUILD_NETWORK"
        -DQTFORGE_BUILD_UI="$BUILD_UI"
        -DCPACK_GENERATOR="$PACKAGE_GENERATORS"
    )

    # Add generator if available
    if [[ -n "$CMAKE_GENERATOR" ]]; then
        CMAKE_ARGS+=($CMAKE_GENERATOR)
    fi

    # Add Qt path if found
    if [[ $QT_FOUND -eq 1 && -n "$QT_DIR" ]]; then
        CMAKE_ARGS+=(-DQt6_DIR="$QT_DIR/lib/cmake/Qt6")
    fi

    # Add toolchain file if specified
    if [[ -n "$CMAKE_TOOLCHAIN" && -f "$SOURCE_DIR/$CMAKE_TOOLCHAIN" ]]; then
        CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN")
        print_status "Using toolchain: $CMAKE_TOOLCHAIN"
    fi

    # MSYS2 specific options
    if [[ "$SYSTEM" == "msys2" ]]; then
        CMAKE_ARGS+=(-DQTFORGE_IS_MSYS2=ON)
        CMAKE_ARGS+=(-DQTFORGE_MSYS2_SUBSYSTEM="$MSYS2_SUBSYSTEM")

        # Add MSYS2 prefix path
        if [[ -n "$MSYSTEM_PREFIX" ]]; then
            CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$MSYSTEM_PREFIX")
        fi

        print_status "MSYS2 configuration: $MSYS2_SUBSYSTEM"
    fi

    # macOS specific options
    if [[ "$SYSTEM" == "macos" ]]; then
        CMAKE_ARGS+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15)
    fi

    # Configure
    print_status "Configuring build..."
    cmake "${CMAKE_ARGS[@]}"

    # Build
    print_status "Building project..."
    cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel "$PARALLEL_JOBS"

    # Run tests if requested
    if [[ "$BUILD_TESTS" == "ON" ]]; then
        print_status "Running tests..."
        ctest --test-dir "$BUILD_DIR" --output-on-failure --parallel "$PARALLEL_JOBS"
    fi

    # Install
    print_status "Installing project..."
    cmake --install "$BUILD_DIR" --config "$BUILD_TYPE"

    # Create packages if requested
    if [[ "$CREATE_PACKAGE" == "ON" ]]; then
        print_status "Creating packages..."
        cd "$BUILD_DIR"
        cpack --config CPackConfig.cmake

        print_status "Packages created in: $BUILD_DIR/packages"
        ls -la "$BUILD_DIR"/packages/ || true
    fi

    print_success "Build completed successfully!"
}

# Run main function
main "$@"
