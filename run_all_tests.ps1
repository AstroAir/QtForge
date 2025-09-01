#!/usr/bin/env pwsh
# QtForge Comprehensive Test Runner
# Runs all available tests with proper environment setup

param(
    [switch]$Verbose = $false,
    [switch]$StopOnFailure = $false
)

# Colors for output
$Red = "`e[31m"
$Green = "`e[32m"
$Yellow = "`e[33m"
$Blue = "`e[34m"
$Reset = "`e[0m"

function Write-TestHeader($message) {
    Write-Host "${Blue}============================================================${Reset}"
    Write-Host "${Blue}$message${Reset}"
    Write-Host "${Blue}============================================================${Reset}"
}

function Write-Success($message) {
    Write-Host "${Green}✅ $message${Reset}"
}

function Write-Error($message) {
    Write-Host "${Red}❌ $message${Reset}"
}

function Write-Warning($message) {
    Write-Host "${Yellow}⚠️  $message${Reset}"
}

# Test results tracking
$TestResults = @()
$TotalTests = 0
$PassedTests = 0
$FailedTests = 0

function Run-Test {
    param(
        [string]$TestName,
        [string]$TestPath,
        [string]$WorkingDirectory = $null,
        [int]$TimeoutSeconds = 60
    )
    
    $global:TotalTests++
    
    Write-Host "`n${Blue}Running: $TestName${Reset}"
    Write-Host "Path: $TestPath"
    
    if ($WorkingDirectory) {
        Write-Host "Working Directory: $WorkingDirectory"
        $OriginalLocation = Get-Location
        Set-Location $WorkingDirectory
    }
    
    try {
        # Set up environment
        $env:PATH += ";D:\Project\QtForge\build\default\Release"
        
        # Run the test
        $Process = Start-Process -FilePath $TestPath -NoNewWindow -Wait -PassThru -RedirectStandardOutput "test_output.tmp" -RedirectStandardError "test_error.tmp"
        
        $ExitCode = $Process.ExitCode
        $Output = ""
        $ErrorOutput = ""
        
        if (Test-Path "test_output.tmp") {
            $Output = Get-Content "test_output.tmp" -Raw
            Remove-Item "test_output.tmp" -ErrorAction SilentlyContinue
        }
        
        if (Test-Path "test_error.tmp") {
            $ErrorOutput = Get-Content "test_error.tmp" -Raw
            Remove-Item "test_error.tmp" -ErrorAction SilentlyContinue
        }
        
        $TestResult = @{
            Name = $TestName
            Path = $TestPath
            ExitCode = $ExitCode
            Output = $Output
            ErrorOutput = $ErrorOutput
            Status = if ($ExitCode -eq 0) { "PASSED" } else { "FAILED" }
        }
        
        $global:TestResults += $TestResult
        
        if ($ExitCode -eq 0) {
            Write-Success "$TestName - PASSED"
            $global:PassedTests++
        } else {
            Write-Error "$TestName - FAILED (Exit Code: $ExitCode)"
            $global:FailedTests++
            
            if ($Verbose -or $ErrorOutput) {
                Write-Host "Error Output:" -ForegroundColor Red
                Write-Host $ErrorOutput
            }
            
            if ($StopOnFailure) {
                throw "Test failed: $TestName"
            }
        }
        
        if ($Verbose -and $Output) {
            Write-Host "Output:" -ForegroundColor Cyan
            Write-Host $Output
        }
        
    } finally {
        if ($WorkingDirectory) {
            Set-Location $OriginalLocation
        }
    }
}

# Main test execution
Write-TestHeader "QtForge Comprehensive Test Suite"

try {
    # Test 1: Basic Plugin Test
    $BasicPluginTest = "D:\Project\QtForge\build\default\examples\01-fundamentals\basic-plugin\Release\BasicPluginTest.exe"
    if (Test-Path $BasicPluginTest) {
        Run-Test "BasicPlugin Core Functionality" $BasicPluginTest
    } else {
        Write-Warning "BasicPluginTest.exe not found"
    }
    
    # Test 2: Service Plugin Test
    $ServicePluginTest = "D:\Project\QtForge\build\default\examples\03-services\background-tasks\Release\ServicePluginTest.exe"
    $ServicePluginWorkDir = "D:\Project\QtForge\build\default\examples\03-services\background-tasks\Release"
    if (Test-Path $ServicePluginTest) {
        Run-Test "ServicePlugin Comprehensive Test" $ServicePluginTest $ServicePluginWorkDir
    } else {
        Write-Warning "ServicePluginTest.exe not found"
    }
    
    # Test 3: Service Plugin Task Test
    $ServicePluginTaskTest = "D:\Project\QtForge\build\default\examples\03-services\background-tasks\Release\ServicePluginTaskTest.exe"
    if (Test-Path $ServicePluginTaskTest) {
        Run-Test "ServicePlugin Task Processing" $ServicePluginTaskTest $ServicePluginWorkDir
    } else {
        Write-Warning "ServicePluginTaskTest.exe not found"
    }
    
    # Summary
    Write-TestHeader "Test Results Summary"
    Write-Host "Total Tests: $TotalTests"
    Write-Success "Passed: $PassedTests"
    Write-Error "Failed: $FailedTests"
    
    if ($FailedTests -eq 0) {
        Write-Success "All tests passed! 🎉"
        exit 0
    } else {
        Write-Error "Some tests failed. See details above."
        exit 1
    }
    
} catch {
    Write-Error "Test execution failed: $_"
    exit 1
}
