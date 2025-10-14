#!/bin/bash
# QtForge Static Analysis Runner
# Version: 1.0
# Purpose: Run static analysis tools on QtForge codebase

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
REPORTS_DIR="${PROJECT_ROOT}/analysis-reports"

# Tool flags
RUN_CLANG_TIDY=false
RUN_CPPCHECK=false
RUN_IWYU=false
RUN_ALL=false
AUTO_FIX=false
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clang-tidy)
            RUN_CLANG_TIDY=true
            shift
            ;;
        --cppcheck)
            RUN_CPPCHECK=true
            shift
            ;;
        --iwyu)
            RUN_IWYU=true
            shift
            ;;
        --all)
            RUN_ALL=true
            shift
            ;;
        --fix)
            AUTO_FIX=true
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --clang-tidy    Run clang-tidy analysis"
            echo "  --cppcheck      Run cppcheck analysis"
            echo "  --iwyu          Run include-what-you-use analysis"
            echo "  --all           Run all analysis tools"
            echo "  --fix           Auto-fix issues (use with caution)"
            echo "  --jobs N        Number of parallel jobs (default: $PARALLEL_JOBS)"
            echo "  --help          Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# If no specific tool selected, show help
if [ "$RUN_ALL" = false ] && [ "$RUN_CLANG_TIDY" = false ] && [ "$RUN_CPPCHECK" = false ] && [ "$RUN_IWYU" = false ]; then
    echo -e "${YELLOW}No analysis tool selected. Use --help for usage information.${NC}"
    exit 1
fi

# Enable all tools if --all is specified
if [ "$RUN_ALL" = true ]; then
    RUN_CLANG_TIDY=true
    RUN_CPPCHECK=true
    RUN_IWYU=true
fi

# Create reports directory
mkdir -p "$REPORTS_DIR"

echo -e "${BLUE}=== QtForge Static Analysis ===${NC}"
echo -e "${BLUE}Project Root: $PROJECT_ROOT${NC}"
echo -e "${BLUE}Build Directory: $BUILD_DIR${NC}"
echo -e "${BLUE}Reports Directory: $REPORTS_DIR${NC}"
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found: $BUILD_DIR${NC}"
    echo -e "${YELLOW}Please build the project first:${NC}"
    echo "  cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    echo "  cmake --build build"
    exit 1
fi

# Check if compile_commands.json exists
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo -e "${RED}Error: compile_commands.json not found${NC}"
    echo -e "${YELLOW}Please rebuild with CMAKE_EXPORT_COMPILE_COMMANDS=ON:${NC}"
    echo "  cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    exit 1
fi

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Run clang-tidy
if [ "$RUN_CLANG_TIDY" = true ]; then
    echo -e "${GREEN}=== Running clang-tidy ===${NC}"

    if ! command_exists clang-tidy; then
        echo -e "${RED}Error: clang-tidy not found${NC}"
        echo "Install with: sudo apt-get install clang-tidy"
        exit 1
    fi

    CLANG_TIDY_ARGS="-p $BUILD_DIR"
    if [ "$AUTO_FIX" = true ]; then
        CLANG_TIDY_ARGS="$CLANG_TIDY_ARGS --fix-errors"
        echo -e "${YELLOW}Warning: Auto-fix enabled. Changes will be applied to source files.${NC}"
    fi

    # Run clang-tidy
    if command_exists run-clang-tidy; then
        echo "Running parallel clang-tidy with $PARALLEL_JOBS jobs..."
        run-clang-tidy -p "$BUILD_DIR" -j "$PARALLEL_JOBS" \
            -checks='-*,clang-analyzer-deadcode.*,misc-unused-*,readability-redundant-*,performance-unnecessary-*' \
            > "$REPORTS_DIR/clang-tidy-report.txt" 2>&1 || true
    else
        echo "Running clang-tidy (sequential)..."
        find "$PROJECT_ROOT/src" "$PROJECT_ROOT/include" -name "*.cpp" -o -name "*.hpp" | \
            xargs clang-tidy $CLANG_TIDY_ARGS \
            > "$REPORTS_DIR/clang-tidy-report.txt" 2>&1 || true
    fi

    echo -e "${GREEN}clang-tidy report saved to: $REPORTS_DIR/clang-tidy-report.txt${NC}"
    echo ""
fi

# Run cppcheck
if [ "$RUN_CPPCHECK" = true ]; then
    echo -e "${GREEN}=== Running cppcheck ===${NC}"

    if ! command_exists cppcheck; then
        echo -e "${RED}Error: cppcheck not found${NC}"
        echo "Install with: sudo apt-get install cppcheck"
        exit 1
    fi

    CPPCHECK_CONFIG="$SCRIPT_DIR/cppcheck.xml"
    if [ ! -f "$CPPCHECK_CONFIG" ]; then
        echo -e "${YELLOW}Warning: cppcheck.xml not found, using default configuration${NC}"
        cppcheck --enable=all --inconclusive \
            --suppress=missingIncludeSystem \
            -I "$PROJECT_ROOT/include" \
            -I "$PROJECT_ROOT/src" \
            --xml \
            --output-file="$REPORTS_DIR/cppcheck-report.xml" \
            "$PROJECT_ROOT/src" "$PROJECT_ROOT/include" 2>&1 || true
    else
        cppcheck --project="$CPPCHECK_CONFIG" \
            --enable=all \
            --inconclusive \
            --suppress=missingIncludeSystem \
            --xml \
            --output-file="$REPORTS_DIR/cppcheck-report.xml" 2>&1 || true
    fi

    echo -e "${GREEN}cppcheck report saved to: $REPORTS_DIR/cppcheck-report.xml${NC}"

    # Generate HTML report if cppcheck-htmlreport is available
    if command_exists cppcheck-htmlreport; then
        echo "Generating HTML report..."
        cppcheck-htmlreport \
            --file="$REPORTS_DIR/cppcheck-report.xml" \
            --report-dir="$REPORTS_DIR/cppcheck-html" \
            --source-dir="$PROJECT_ROOT"
        echo -e "${GREEN}HTML report saved to: $REPORTS_DIR/cppcheck-html/index.html${NC}"
    fi
    echo ""
fi

# Run include-what-you-use
if [ "$RUN_IWYU" = true ]; then
    echo -e "${GREEN}=== Running include-what-you-use ===${NC}"

    if ! command_exists include-what-you-use; then
        echo -e "${RED}Error: include-what-you-use not found${NC}"
        echo "Install with: sudo apt-get install iwyu"
        exit 1
    fi

    IWYU_MAPPINGS="$SCRIPT_DIR/iwyu_mappings.imp"
    IWYU_ARGS=""
    if [ -f "$IWYU_MAPPINGS" ]; then
        IWYU_ARGS="-Xiwyu --mapping_file=$IWYU_MAPPINGS"
    else
        echo -e "${YELLOW}Warning: iwyu_mappings.imp not found, using default mappings${NC}"
    fi

    if command_exists iwyu_tool.py; then
        echo "Running IWYU analysis..."
        iwyu_tool.py -p "$BUILD_DIR" -- $IWYU_ARGS \
            > "$REPORTS_DIR/iwyu-report.txt" 2>&1 || true
    else
        echo -e "${YELLOW}Warning: iwyu_tool.py not found, running on individual files${NC}"
        find "$PROJECT_ROOT/src" -name "*.cpp" | head -10 | \
            xargs -I {} include-what-you-use -p "$BUILD_DIR" $IWYU_ARGS {} \
            > "$REPORTS_DIR/iwyu-report.txt" 2>&1 || true
    fi

    echo -e "${GREEN}IWYU report saved to: $REPORTS_DIR/iwyu-report.txt${NC}"

    if [ "$AUTO_FIX" = true ] && command_exists fix_includes.py; then
        echo -e "${YELLOW}Applying IWYU fixes...${NC}"
        fix_includes.py < "$REPORTS_DIR/iwyu-report.txt" || true
        echo -e "${GREEN}IWYU fixes applied${NC}"
    fi
    echo ""
fi

# Summary
echo -e "${BLUE}=== Analysis Complete ===${NC}"
echo -e "${GREEN}Reports saved to: $REPORTS_DIR${NC}"
echo ""
echo "Next steps:"
echo "  1. Review the reports in $REPORTS_DIR"
echo "  2. Verify findings manually before making changes"
echo "  3. Apply fixes incrementally and test after each change"
echo ""
echo -e "${YELLOW}Remember: Always verify tool output manually!${NC}"
