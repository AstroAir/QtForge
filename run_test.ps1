#!/usr/bin/env powershell
<#
.SYNOPSIS
    Simple QtForge Test Runner Script for CTest Integration

.PARAMETER TestExecutable
    Path to the test executable to run

.PARAMETER WorkingDirectory
    Working directory for test execution
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$TestExecutable,

    [Parameter(Mandatory=$false)]
    [string]$WorkingDirectory = "."
)

$ErrorActionPreference = "Stop"

try {
    Write-Host "QtForge Test Runner - Simple Version" -ForegroundColor Green
    Write-Host "Executable: $TestExecutable" -ForegroundColor Cyan
    Write-Host "Working Directory: $WorkingDirectory" -ForegroundColor Cyan

    # Validate executable exists
    if (-not (Test-Path $TestExecutable)) {
        Write-Host "ERROR: Test executable not found: $TestExecutable" -ForegroundColor Red
        exit 1
    }

    # Go to build directory (up from tests/core/test.exe)
    $BuildDirectory = Split-Path (Split-Path (Split-Path $TestExecutable -Parent) -Parent) -Parent
    if (Test-Path $BuildDirectory) {
        Push-Location $BuildDirectory
        Write-Host "Running from build directory: $(Get-Location)" -ForegroundColor Cyan
    }

    # Add Qt bin to PATH
    $env:PATH = "D:\msys64\mingw64\bin;" + $env:PATH

    Write-Host "Executing test..." -ForegroundColor Yellow
    Write-Host "===========================================" -ForegroundColor Green

    # Execute the test directly
    & $TestExecutable
    $ExitCode = $LASTEXITCODE

    Write-Host "===========================================" -ForegroundColor Green
    Write-Host "Test completed with exit code: $ExitCode" -ForegroundColor $(if ($ExitCode -eq 0) { "Green" } else { "Red" })

    exit $ExitCode

} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location -ErrorAction SilentlyContinue
}
