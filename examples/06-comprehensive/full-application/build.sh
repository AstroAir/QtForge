#!/bin/bash

# QtForge Comprehensive Example Build Script
# This script builds the comprehensive example demonstrating all QtForge features

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
PARALLEL_JOBS=${PARALLEL_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}

# Feature flags
BUILD_TESTS=${BUILD_TESTS:-ON}
BUILD_DOCUMENTATION=${BUILD_DOCUMENTATION:-OFF}
QTFORGE_PYTHON_SUPPORT=${QTFORGE_PYTHON_SUPPORT:-OFF}
ENABLE_COVERAGE=${ENABLE_COVERAGE:-OFF}

# Functions
print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    print_header "Checking Dependencies"

    # Check for required tools
    local missing_deps=()

    if ! command -v cmake >/dev/null 2>&1; then
        missing_deps+=("cmake")
    fi

    if ! command -v make >/dev/null 2>&1 && ! command -v ninja >/dev/null 2>&1; then
        missing_deps+=("make or ninja")
    fi

    if ! command -v pkg-config >/dev/null 2>&1; then
        missing_deps+=("pkg-config")
    fi

    # Check Qt6
    if ! pkg-config --exists Qt6Core; then
        missing_deps+=("Qt6")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing dependencies and try again."
        exit 1
    fi

    print_info "All dependencies found"

    # Print versions
    echo "Tool versions:"
    echo "  CMake: $(cmake --version | head -n1)"
    echo "  Qt6: $(pkg-config --modversion Qt6Core)"

    if command -v python3 >/dev/null 2>&1; then
        echo "  Python: $(python3 --version)"
    fi
}

configure_build() {
    print_header "Configuring Build"

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Determine generator
    local generator=""
    if command -v ninja >/dev/null 2>&1; then
        generator="-G Ninja"
        print_info "Using Ninja generator"
    else
        print_info "Using Make generator"
    fi

    # CMake configuration
    print_info "Running CMake configuration..."
    cmake .. $generator \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DBUILD_TESTING="$BUILD_TESTS" \
        -DBUILD_DOCUMENTATION="$BUILD_DOCUMENTATION" \
        -DQTFORGE_PYTHON_SUPPORT="$QTFORGE_PYTHON_SUPPORT" \
        -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    print_info "Configuration completed successfully"
}

build_project() {
    print_header "Building Project"

    print_info "Building with $PARALLEL_JOBS parallel jobs..."
    cmake --build . --parallel "$PARALLEL_JOBS"

    print_info "Build completed successfully"
}

run_tests() {
    if [ "$BUILD_TESTS" = "ON" ]; then
        print_header "Running Tests"

        print_info "Running test suite..."
        ctest --output-on-failure --parallel "$PARALLEL_JOBS"

        print_info "All tests passed"
    else
        print_info "Tests disabled, skipping test execution"
    fi
}

generate_documentation() {
    if [ "$BUILD_DOCUMENTATION" = "ON" ]; then
        print_header "Generating Documentation"

        if command -v doxygen >/dev/null 2>&1; then
            cmake --build . --target doc_comprehensive_example
            print_info "Documentation generated successfully"
        else
            print_warning "Doxygen not found, skipping documentation generation"
        fi
    else
        print_info "Documentation generation disabled"
    fi
}

install_project() {
    if [ "$1" = "--install" ]; then
        print_header "Installing Project"

        print_info "Installing to $INSTALL_PREFIX..."
        cmake --install . --prefix "$INSTALL_PREFIX"

        print_info "Installation completed successfully"
    fi
}

create_package() {
    if [ "$1" = "--package" ]; then
        print_header "Creating Package"

        print_info "Creating distribution package..."
        cpack

        print_info "Package created successfully"
    fi
}

run_demo() {
    if [ "$1" = "--run" ]; then
        print_header "Running Demo"

        print_info "Starting comprehensive demo..."

        # Ensure plugins directory exists
        mkdir -p plugins

        # Run the demo
        ./comprehensive_demo --plugin-dir=plugins --enable-python="$QTFORGE_PYTHON_SUPPORT"

        print_info "Demo completed"
    fi
}

print_summary() {
    print_header "Build Summary"

    echo "Configuration:"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Build Directory: $BUILD_DIR"
    echo "  Install Prefix: $INSTALL_PREFIX"
    echo "  Parallel Jobs: $PARALLEL_JOBS"
    echo "  Tests: $BUILD_TESTS"
    echo "  Documentation: $BUILD_DOCUMENTATION"
    echo "  Python Support: $QTFORGE_PYTHON_SUPPORT"
    echo "  Coverage: $ENABLE_COVERAGE"
    echo ""

    echo "Build artifacts:"
    if [ -f "comprehensive_demo" ]; then
        echo "  ✅ Main application: comprehensive_demo"
    fi

    if [ -f "plugins/comprehensive_plugin.qtplugin" ]; then
        echo "  ✅ Comprehensive plugin: plugins/comprehensive_plugin.qtplugin"
    fi

    if [ -f "test_comprehensive_plugin" ]; then
        echo "  ✅ Test executable: test_comprehensive_plugin"
    fi

    echo ""
    echo "Usage:"
    echo "  Run demo:           ./comprehensive_demo"
    echo "  Run with options:   ./comprehensive_demo --plugin-dir=plugins --security-level=high"
    echo "  Run tests:          ctest"
    echo "  Run Python demo:    python3 ../python/comprehensive_demo.py"
    echo ""

    if [ "$QTFORGE_PYTHON_SUPPORT" = "ON" ]; then
        echo "Python integration:"
        echo "  Python demo:        python3 ../python/comprehensive_demo.py"
        echo "  Python path:        export PYTHONPATH=\$PYTHONPATH:$(pwd)"
    fi
}

show_help() {
    echo "QtForge Comprehensive Example Build Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --help              Show this help message"
    echo "  --clean             Clean build directory before building"
    echo "  --install           Install after building"
    echo "  --package           Create distribution package"
    echo "  --run               Run demo after building"
    echo "  --debug             Build in Debug mode"
    echo "  --release           Build in Release mode (default)"
    echo "  --with-python       Enable Python support"
    echo "  --with-docs         Enable documentation generation"
    echo "  --with-coverage     Enable code coverage"
    echo "  --no-tests          Disable tests"
    echo ""
    echo "Environment variables:"
    echo "  BUILD_TYPE          Build type (Debug|Release)"
    echo "  BUILD_DIR           Build directory (default: build)"
    echo "  INSTALL_PREFIX      Install prefix (default: /usr/local)"
    echo "  PARALLEL_JOBS       Number of parallel jobs"
    echo ""
    echo "Examples:"
    echo "  $0                          # Basic build"
    echo "  $0 --debug --with-python    # Debug build with Python"
    echo "  $0 --install --package      # Build, install, and package"
    echo "  $0 --clean --run            # Clean build and run demo"
}

# Parse command line arguments
CLEAN_BUILD=false
INSTALL_PROJECT=false
CREATE_PACKAGE=false
RUN_DEMO=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --install)
            INSTALL_PROJECT=true
            shift
            ;;
        --package)
            CREATE_PACKAGE=true
            shift
            ;;
        --run)
            RUN_DEMO=true
            shift
            ;;
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --release)
            BUILD_TYPE=Release
            shift
            ;;
        --with-python)
            QTFORGE_PYTHON_SUPPORT=ON
            shift
            ;;
        --with-docs)
            BUILD_DOCUMENTATION=ON
            shift
            ;;
        --with-coverage)
            ENABLE_COVERAGE=ON
            shift
            ;;
        --no-tests)
            BUILD_TESTS=OFF
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Main execution
main() {
    print_header "QtForge Comprehensive Example Build"

    # Clean build if requested
    if [ "$CLEAN_BUILD" = true ]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi

    # Execute build steps
    check_dependencies
    configure_build
    build_project
    run_tests
    generate_documentation

    # Optional steps
    if [ "$INSTALL_PROJECT" = true ]; then
        install_project --install
    fi

    if [ "$CREATE_PACKAGE" = true ]; then
        create_package --package
    fi

    if [ "$RUN_DEMO" = true ]; then
        run_demo --run
    fi

    print_summary

    print_info "Build script completed successfully!"
}

# Run main function
main "$@"
