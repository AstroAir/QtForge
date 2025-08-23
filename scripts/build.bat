@echo off
REM QtPlugin Windows Build Script
REM Automates building and packaging for Windows

setlocal enabledelayedexpansion

REM Default configuration
set BUILD_TYPE=Release
set BUILD_DIR=%~dp0..\build
set INSTALL_DIR=%~dp0..\install
set SOURCE_DIR=%~dp0..
set PARALLEL_JOBS=%NUMBER_OF_PROCESSORS%
set BUILD_TESTS=OFF
set BUILD_EXAMPLES=ON
set BUILD_NETWORK=OFF
set BUILD_UI=OFF
set CREATE_PACKAGE=OFF
set CLEAN_BUILD=OFF

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--help" goto :show_help
if /i "%~1"=="-h" goto :show_help
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="--release" set BUILD_TYPE=Release
if /i "%~1"=="--tests" set BUILD_TESTS=ON
if /i "%~1"=="--network" set BUILD_NETWORK=ON
if /i "%~1"=="--ui" set BUILD_UI=ON
if /i "%~1"=="--package" set CREATE_PACKAGE=ON
if /i "%~1"=="--clean" set CLEAN_BUILD=ON
if /i "%~1"=="--jobs" (
    shift
    set PARALLEL_JOBS=%~1
)
shift
goto :parse_args

:args_done

echo.
echo ========================================
echo QtPlugin Windows Build Script
echo ========================================
echo Build Type: %BUILD_TYPE%
echo Build Directory: %BUILD_DIR%
echo Install Directory: %INSTALL_DIR%
echo Parallel Jobs: %PARALLEL_JOBS%
echo Build Tests: %BUILD_TESTS%
echo Build Examples: %BUILD_EXAMPLES%
echo Build Network: %BUILD_NETWORK%
echo Build UI: %BUILD_UI%
echo Create Package: %CREATE_PACKAGE%
echo ========================================
echo.

REM Clean build directory if requested
if /i "%CLEAN_BUILD%"=="ON" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

REM Detect Visual Studio
call :detect_vs
if !VS_FOUND!==0 (
    echo Warning: Visual Studio not detected, using default generator
    set CMAKE_GENERATOR=
) else (
    echo Found Visual Studio: !VS_VERSION!
    set CMAKE_GENERATOR=-G "!VS_GENERATOR!"
)

REM Detect Qt
call :detect_qt
if !QT_FOUND!==0 (
    echo Warning: Qt6 not found in standard locations
    set QT_CMAKE_ARG=
) else (
    echo Found Qt6 at: !QT_DIR!
    set QT_CMAKE_ARG=-DQt6_DIR=!QT_DIR!\lib\cmake\Qt6
)

REM Configure with CMake
echo.
echo Configuring build...
cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" ^
    %CMAKE_GENERATOR% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DQTPLUGIN_BUILD_TESTS=%BUILD_TESTS% ^
    -DQTPLUGIN_BUILD_EXAMPLES=%BUILD_EXAMPLES% ^
    -DQTPLUGIN_BUILD_NETWORK=%BUILD_NETWORK% ^
    -DQTPLUGIN_BUILD_UI=%BUILD_UI% ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL ^
    -DCPACK_GENERATOR="NSIS;ZIP;WIX" ^
    %QT_CMAKE_ARG%

if errorlevel 1 (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo.
echo Building project...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel %PARALLEL_JOBS%

if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM Run tests if requested
if /i "%BUILD_TESTS%"=="ON" (
    echo.
    echo Running tests...
    ctest --test-dir "%BUILD_DIR%" --output-on-failure --parallel %PARALLEL_JOBS%
    
    if errorlevel 1 (
        echo Tests failed!
        exit /b 1
    )
)

REM Install
echo.
echo Installing project...
cmake --install "%BUILD_DIR%" --config %BUILD_TYPE%

if errorlevel 1 (
    echo Installation failed!
    exit /b 1
)

REM Create packages if requested
if /i "%CREATE_PACKAGE%"=="ON" (
    echo.
    echo Creating packages...
    cd /d "%BUILD_DIR%"
    cpack --config CPackConfig.cmake
    
    if errorlevel 1 (
        echo Packaging failed!
        exit /b 1
    )
    
    echo.
    echo Packages created in: %BUILD_DIR%\packages
    dir "%BUILD_DIR%\packages" /b
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
goto :eof

REM Function to detect Visual Studio
:detect_vs
set VS_FOUND=0
set VS_VERSION=
set VS_GENERATOR=

REM Check for VS 2022
where devenv.exe >nul 2>&1
if !errorlevel!==0 (
    for /f "tokens=*" %%i in ('where devenv.exe') do (
        echo %%i | findstr "2022" >nul
        if !errorlevel!==0 (
            set VS_FOUND=1
            set VS_VERSION=2022
            set VS_GENERATOR=Visual Studio 17 2022
            goto :vs_done
        )
    )
)

REM Check for VS 2019
reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\16.0" >nul 2>&1
if !errorlevel!==0 (
    set VS_FOUND=1
    set VS_VERSION=2019
    set VS_GENERATOR=Visual Studio 16 2019
    goto :vs_done
)

REM Check for VS 2017
reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\15.0" >nul 2>&1
if !errorlevel!==0 (
    set VS_FOUND=1
    set VS_VERSION=2017
    set VS_GENERATOR=Visual Studio 15 2017
)

:vs_done
goto :eof

REM Function to detect Qt
:detect_qt
set QT_FOUND=0
set QT_DIR=

REM Check environment variables
if defined Qt6_DIR (
    if exist "%Qt6_DIR%" (
        set QT_FOUND=1
        set QT_DIR=%Qt6_DIR%
        goto :qt_done
    )
)

if defined QT_DIR (
    if exist "%QT_DIR%" (
        set QT_FOUND=1
        set QT_DIR=%QT_DIR%
        goto :qt_done
    )
)

REM Check common installation paths
set QT_PATHS=C:\Qt\6.6.0\msvc2022_64 C:\Qt\6.5.0\msvc2022_64 C:\Qt\6.4.0\msvc2022_64

for %%p in (%QT_PATHS%) do (
    if exist "%%p" (
        set QT_FOUND=1
        set QT_DIR=%%p
        goto :qt_done
    )
)

:qt_done
goto :eof

:show_help
echo QtPlugin Windows Build Script
echo.
echo Usage: build.bat [options]
echo.
echo Options:
echo   --help, -h      Show this help message
echo   --debug         Build in Debug mode (default: Release)
echo   --release       Build in Release mode
echo   --tests         Build and run tests
echo   --network       Build network support
echo   --ui            Build UI support
echo   --package       Create packages after build
echo   --clean         Clean build directory before building
echo   --jobs N        Number of parallel jobs (default: CPU count)
echo.
echo Examples:
echo   build.bat                           # Basic release build
echo   build.bat --debug --tests           # Debug build with tests
echo   build.bat --release --package       # Release build with packaging
echo   build.bat --clean --network --ui    # Clean build with all features
echo.
goto :eof
