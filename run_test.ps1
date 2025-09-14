#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Test runner script for QtForge project
.DESCRIPTION
    This script runs test executables with proper environment setup and error handling.
.PARAMETER TestExecutable
    Path to the test executable to run
.PARAMETER WorkingDirectory
    Working directory for the test execution
.PARAMETER Timeout
    Timeout in seconds (optional)
.EXAMPLE
    .\run_test.ps1 -TestExecutable "build/tests/core/test_plugin_interface.exe" -WorkingDirectory "build/tests/core"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$TestExecutable,
    
    [Parameter(Mandatory=$true)]
    [string]$WorkingDirectory,
    
    [Parameter(Mandatory=$false)]
    [int]$Timeout = 300
)

# Set error action preference
$ErrorActionPreference = "Stop"

try {
    # Validate parameters
    if (-not (Test-Path $TestExecutable)) {
        Write-Error "Test executable not found: $TestExecutable"
        exit 1
    }
    
    if (-not (Test-Path $WorkingDirectory)) {
        Write-Error "Working directory not found: $WorkingDirectory"
        exit 1
    }
    
    # Change to working directory
    Push-Location $WorkingDirectory
    
    # Set up environment variables for Qt and QtForge
    $env:QT_QPA_PLATFORM = "offscreen"  # Use offscreen platform for headless testing
    $env:QT_LOGGING_RULES = "*.debug=false"  # Reduce log noise unless debugging
    
    # Add the build directory to PATH so DLLs can be found
    $buildDir = Split-Path -Parent (Split-Path -Parent $WorkingDirectory)
    $env:PATH = "$buildDir;$env:PATH"
    
    Write-Host "Running test: $TestExecutable"
    Write-Host "Working directory: $WorkingDirectory"
    Write-Host "Timeout: $Timeout seconds"
    
    # Run the test executable with timeout
    $process = Start-Process -FilePath $TestExecutable -NoNewWindow -PassThru -Wait
    
    # Check exit code
    if ($process.ExitCode -eq 0) {
        Write-Host "Test passed successfully" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "Test failed with exit code: $($process.ExitCode)" -ForegroundColor Red
        exit $process.ExitCode
    }
    
} catch {
    Write-Error "Error running test: $_"
    exit 1
} finally {
    # Restore original location
    Pop-Location
}
