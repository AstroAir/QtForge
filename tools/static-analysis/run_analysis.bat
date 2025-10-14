@echo off
REM QtForge Static Analysis Runner (Windows)
REM Version: 1.0
REM Purpose: Run static analysis tools on QtForge codebase

setlocal enabledelayedexpansion

REM Configuration
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\..\"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "REPORTS_DIR=%PROJECT_ROOT%analysis-reports"

REM Tool flags
set RUN_CLANG_TIDY=0
set RUN_CPPCHECK=0
set RUN_IWYU=0
set RUN_ALL=0
set AUTO_FIX=0
set PARALLEL_JOBS=%NUMBER_OF_PROCESSORS%

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_parse_args
if /i "%~1"=="--clang-tidy" (
    set RUN_CLANG_TIDY=1
    shift
    goto parse_args
)
if /i "%~1"=="--cppcheck" (
    set RUN_CPPCHECK=1
    shift
    goto parse_args
)
if /i "%~1"=="--iwyu" (
    set RUN_IWYU=1
    shift
    goto parse_args
)
if /i "%~1"=="--all" (
    set RUN_ALL=1
    shift
    goto parse_args
)
if /i "%~1"=="--fix" (
    set AUTO_FIX=1
    shift
    goto parse_args
)
if /i "%~1"=="--jobs" (
    set PARALLEL_JOBS=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    echo Usage: %~nx0 [OPTIONS]
    echo.
    echo Options:
    echo   --clang-tidy    Run clang-tidy analysis
    echo   --cppcheck      Run cppcheck analysis
    echo   --iwyu          Run include-what-you-use analysis
    echo   --all           Run all analysis tools
    echo   --fix           Auto-fix issues (use with caution^)
    echo   --jobs N        Number of parallel jobs (default: %NUMBER_OF_PROCESSORS%^)
    echo   --help          Show this help message
    exit /b 0
)
echo Unknown option: %~1
exit /b 1

:end_parse_args

REM If no specific tool selected, show help
if %RUN_ALL%==0 if %RUN_CLANG_TIDY%==0 if %RUN_CPPCHECK%==0 if %RUN_IWYU%==0 (
    echo No analysis tool selected. Use --help for usage information.
    exit /b 1
)

REM Enable all tools if --all is specified
if %RUN_ALL%==1 (
    set RUN_CLANG_TIDY=1
    set RUN_CPPCHECK=1
    set RUN_IWYU=1
)

REM Create reports directory
if not exist "%REPORTS_DIR%" mkdir "%REPORTS_DIR%"

echo === QtForge Static Analysis ===
echo Project Root: %PROJECT_ROOT%
echo Build Directory: %BUILD_DIR%
echo Reports Directory: %REPORTS_DIR%
echo.

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo Error: Build directory not found: %BUILD_DIR%
    echo Please build the project first:
    echo   cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    echo   cmake --build build
    exit /b 1
)

REM Check if compile_commands.json exists
if not exist "%BUILD_DIR%\compile_commands.json" (
    echo Error: compile_commands.json not found
    echo Please rebuild with CMAKE_EXPORT_COMPILE_COMMANDS=ON:
    echo   cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    exit /b 1
)

REM Run clang-tidy
if %RUN_CLANG_TIDY%==1 (
    echo === Running clang-tidy ===

    where clang-tidy >nul 2>&1
    if errorlevel 1 (
        echo Error: clang-tidy not found
        echo Install LLVM from https://llvm.org/builds/
        exit /b 1
    )

    set CLANG_TIDY_ARGS=-p "%BUILD_DIR%"
    if %AUTO_FIX%==1 (
        set CLANG_TIDY_ARGS=!CLANG_TIDY_ARGS! --fix-errors
        echo Warning: Auto-fix enabled. Changes will be applied to source files.
    )

    echo Running clang-tidy...
    for /r "%PROJECT_ROOT%\src" %%f in (*.cpp) do (
        clang-tidy !CLANG_TIDY_ARGS! "%%f" >> "%REPORTS_DIR%\clang-tidy-report.txt" 2>&1
    )

    echo clang-tidy report saved to: %REPORTS_DIR%\clang-tidy-report.txt
    echo.
)

REM Run cppcheck
if %RUN_CPPCHECK%==1 (
    echo === Running cppcheck ===

    where cppcheck >nul 2>&1
    if errorlevel 1 (
        echo Error: cppcheck not found
        echo Install from https://cppcheck.sourceforge.io/
        exit /b 1
    )

    set CPPCHECK_CONFIG=%SCRIPT_DIR%cppcheck.xml
    if exist "!CPPCHECK_CONFIG!" (
        cppcheck --project="!CPPCHECK_CONFIG!" --enable=all --inconclusive --suppress=missingIncludeSystem --xml --output-file="%REPORTS_DIR%\cppcheck-report.xml" 2>&1
    ) else (
        echo Warning: cppcheck.xml not found, using default configuration
        cppcheck --enable=all --inconclusive --suppress=missingIncludeSystem -I "%PROJECT_ROOT%\include" -I "%PROJECT_ROOT%\src" --xml --output-file="%REPORTS_DIR%\cppcheck-report.xml" "%PROJECT_ROOT%\src" "%PROJECT_ROOT%\include" 2>&1
    )

    echo cppcheck report saved to: %REPORTS_DIR%\cppcheck-report.xml
    echo.
)

REM Run include-what-you-use
if %RUN_IWYU%==1 (
    echo === Running include-what-you-use ===

    where include-what-you-use >nul 2>&1
    if errorlevel 1 (
        echo Error: include-what-you-use not found
        echo IWYU is not commonly available on Windows
        echo Consider using WSL or Linux for IWYU analysis
        exit /b 1
    )

    echo Running IWYU analysis...
    REM IWYU implementation for Windows (if available)
    echo IWYU analysis not fully supported on Windows
    echo.
)

REM Summary
echo === Analysis Complete ===
echo Reports saved to: %REPORTS_DIR%
echo.
echo Next steps:
echo   1. Review the reports in %REPORTS_DIR%
echo   2. Verify findings manually before making changes
echo   3. Apply fixes incrementally and test after each change
echo.
echo Remember: Always verify tool output manually!

endlocal
