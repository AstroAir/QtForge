@echo off
REM QtForge Comprehensive Example Build Script for Windows
REM This script builds the comprehensive example demonstrating all QtForge features

setlocal enabledelayedexpansion

REM Configuration
set BUILD_TYPE=Release
set BUILD_DIR=build
set INSTALL_PREFIX=C:\QtForge
set PARALLEL_JOBS=4

REM Feature flags
set BUILD_TESTS=ON
set BUILD_DOCUMENTATION=OFF
set QTFORGE_PYTHON_SUPPORT=OFF
set ENABLE_COVERAGE=OFF

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :start_build
if "%~1"=="--help" goto :show_help
if "%~1"=="--clean" set CLEAN_BUILD=true
if "%~1"=="--install" set INSTALL_PROJECT=true
if "%~1"=="--package" set CREATE_PACKAGE=true
if "%~1"=="--run" set RUN_DEMO=true
if "%~1"=="--debug" set BUILD_TYPE=Debug
if "%~1"=="--release" set BUILD_TYPE=Release
if "%~1"=="--with-python" set QTFORGE_PYTHON_SUPPORT=ON
if "%~1"=="--with-docs" set BUILD_DOCUMENTATION=ON
if "%~1"=="--with-coverage" set ENABLE_COVERAGE=ON
if "%~1"=="--no-tests" set BUILD_TESTS=OFF
shift
goto :parse_args

:show_help
echo QtForge Comprehensive Example Build Script for Windows
echo.
echo Usage: %0 [options]
echo.
echo Options:
echo   --help              Show this help message
echo   --clean             Clean build directory before building
echo   --install           Install after building
echo   --package           Create distribution package
echo   --run               Run demo after building
echo   --debug             Build in Debug mode
echo   --release           Build in Release mode (default)
echo   --with-python       Enable Python support
echo   --with-docs         Enable documentation generation
echo   --with-coverage     Enable code coverage
echo   --no-tests          Disable tests
echo.
echo Examples:
echo   %0                          # Basic build
echo   %0 --debug --with-python    # Debug build with Python
echo   %0 --install --package      # Build, install, and package
echo   %0 --clean --run            # Clean build and run demo
goto :eof

:start_build
echo ================================
echo QtForge Comprehensive Example Build
echo ================================
echo.

REM Check dependencies
echo [INFO] Checking dependencies...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found. Please install CMake and add it to PATH.
    exit /b 1
)

where msbuild >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSBuild not found. Please run from Visual Studio Developer Command Prompt.
    exit /b 1
)

echo [INFO] All dependencies found
echo Tool versions:
cmake --version | findstr /C:"cmake version"

REM Clean build if requested
if "%CLEAN_BUILD%"=="true" (
    echo [INFO] Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Configure build
echo ================================
echo Configuring Build
echo ================================
echo.

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo [INFO] Running CMake configuration...
cmake .. ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
    -DBUILD_TESTING=%BUILD_TESTS% ^
    -DBUILD_DOCUMENTATION=%BUILD_DOCUMENTATION% ^
    -DQTFORGE_PYTHON_SUPPORT=%QTFORGE_PYTHON_SUPPORT% ^
    -DENABLE_COVERAGE=%ENABLE_COVERAGE% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if errorlevel 1 (
    echo [ERROR] CMake configuration failed
    exit /b 1
)

echo [INFO] Configuration completed successfully

REM Build project
echo ================================
echo Building Project
echo ================================
echo.

echo [INFO] Building with %PARALLEL_JOBS% parallel jobs...
cmake --build . --config %BUILD_TYPE% --parallel %PARALLEL_JOBS%

if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo [INFO] Build completed successfully

REM Run tests
if "%BUILD_TESTS%"=="ON" (
    echo ================================
    echo Running Tests
    echo ================================
    echo.

    echo [INFO] Running test suite...
    ctest --output-on-failure --parallel %PARALLEL_JOBS% --build-config %BUILD_TYPE%

    if errorlevel 1 (
        echo [WARNING] Some tests failed
    ) else (
        echo [INFO] All tests passed
    )
) else (
    echo [INFO] Tests disabled, skipping test execution
)

REM Generate documentation
if "%BUILD_DOCUMENTATION%"=="ON" (
    echo ================================
    echo Generating Documentation
    echo ================================
    echo.

    where doxygen >nul 2>&1
    if errorlevel 1 (
        echo [WARNING] Doxygen not found, skipping documentation generation
    ) else (
        cmake --build . --target doc_comprehensive_example --config %BUILD_TYPE%
        echo [INFO] Documentation generated successfully
    )
) else (
    echo [INFO] Documentation generation disabled
)

REM Install project
if "%INSTALL_PROJECT%"=="true" (
    echo ================================
    echo Installing Project
    echo ================================
    echo.

    echo [INFO] Installing to %INSTALL_PREFIX%...
    cmake --install . --prefix "%INSTALL_PREFIX%" --config %BUILD_TYPE%

    if errorlevel 1 (
        echo [ERROR] Installation failed
        exit /b 1
    )

    echo [INFO] Installation completed successfully
)

REM Create package
if "%CREATE_PACKAGE%"=="true" (
    echo ================================
    echo Creating Package
    echo ================================
    echo.

    echo [INFO] Creating distribution package...
    cpack --config CPackConfig.cmake

    if errorlevel 1 (
        echo [ERROR] Package creation failed
        exit /b 1
    )

    echo [INFO] Package created successfully
)

REM Run demo
if "%RUN_DEMO%"=="true" (
    echo ================================
    echo Running Demo
    echo ================================
    echo.

    echo [INFO] Starting comprehensive demo...

    REM Ensure plugins directory exists
    if not exist "plugins" mkdir "plugins"

    REM Run the demo
    if "%BUILD_TYPE%"=="Debug" (
        comprehensive_demo_d.exe --plugin-dir=plugins --enable-python=%QTFORGE_PYTHON_SUPPORT%
    ) else (
        comprehensive_demo.exe --plugin-dir=plugins --enable-python=%QTFORGE_PYTHON_SUPPORT%
    )

    echo [INFO] Demo completed
)

REM Print summary
echo ================================
echo Build Summary
echo ================================
echo.

echo Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Build Directory: %BUILD_DIR%
echo   Install Prefix: %INSTALL_PREFIX%
echo   Parallel Jobs: %PARALLEL_JOBS%
echo   Tests: %BUILD_TESTS%
echo   Documentation: %BUILD_DOCUMENTATION%
echo   Python Support: %QTFORGE_PYTHON_SUPPORT%
echo   Coverage: %ENABLE_COVERAGE%
echo.

echo Build artifacts:
if exist "comprehensive_demo.exe" echo   ✅ Main application: comprehensive_demo.exe
if exist "comprehensive_demo_d.exe" echo   ✅ Main application (Debug): comprehensive_demo_d.exe
if exist "plugins\comprehensive_plugin.qtplugin" echo   ✅ Comprehensive plugin: plugins\comprehensive_plugin.qtplugin
if exist "test_comprehensive_plugin.exe" echo   ✅ Test executable: test_comprehensive_plugin.exe

echo.
echo Usage:
echo   Run demo:           comprehensive_demo.exe
echo   Run with options:   comprehensive_demo.exe --plugin-dir=plugins --security-level=high
echo   Run tests:          ctest
if "%QTFORGE_PYTHON_SUPPORT%"=="ON" (
    echo   Run Python demo:    python ..\python\comprehensive_demo.py
)

echo.
if "%QTFORGE_PYTHON_SUPPORT%"=="ON" (
    echo Python integration:
    echo   Python demo:        python ..\python\comprehensive_demo.py
    echo   Python path:        set PYTHONPATH=%%PYTHONPATH%%;%cd%
)

echo [INFO] Build script completed successfully!

cd ..
goto :eof
