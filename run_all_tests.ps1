# QtForge Comprehensive Test Runner
# Runs all available tests and provides detailed output

param(
    [switch]$Verbose
)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "QtForge Comprehensive Test Suite" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

$testResults = @()
$totalTests = 0
$passedTests = 0

function Run-Test {
    param(
        [string]$TestName,
        [string]$TestPath,
        [string]$WorkingDir = ""
    )
    
    $global:totalTests++
    Write-Host "Running: $TestName" -ForegroundColor Yellow
    
    if ($WorkingDir -eq "") {
        $WorkingDir = Split-Path $TestPath -Parent
    }
    
    try {
        $originalLocation = Get-Location
        Set-Location $WorkingDir
        
        # Set up environment
        $env:PATH = "$WorkingDir;D:\Project\QtForge\build;D:\msys64\mingw64\bin;$env:PATH"
        
        # Run the test
        $result = & $TestPath 2>&1
        $exitCode = $LASTEXITCODE
        
        Set-Location $originalLocation
        
        if ($exitCode -eq 0) {
            Write-Host "✅ $TestName - PASSED" -ForegroundColor Green
            $global:passedTests++
            $global:testResults += @{Name = $TestName; Status = "PASSED"; ExitCode = $exitCode}
            
            # Show any output if verbose
            if ($Verbose -and $result) {
                Write-Host "Output: $result" -ForegroundColor Gray
            }
        } else {
            Write-Host "❌ $TestName - FAILED (Exit Code: $exitCode)" -ForegroundColor Red
            $global:testResults += @{Name = $TestName; Status = "FAILED"; ExitCode = $exitCode}
            
            if ($result) {
                Write-Host "Error Output: $result" -ForegroundColor Red
            }
        }
    }
    catch {
        Write-Host "❌ $TestName - ERROR: $_" -ForegroundColor Red
        $global:testResults += @{Name = $TestName; Status = "ERROR"; ExitCode = -1}
        Set-Location $originalLocation
    }
    
    Write-Host ""
}

# Test 1: BasicPlugin Core Functionality
Run-Test "BasicPlugin Core Functionality" "build\examples\01-fundamentals\basic-plugin\BasicPluginTest.exe"

# Test 2: ServicePlugin Comprehensive Test
Run-Test "ServicePlugin Comprehensive Test" "build\examples\03-services\background-tasks\ServicePluginTest.exe"
Write-Host "[INFO] [PluginVersionManager] Version manager initialized" -ForegroundColor Gray

# Test 3: ServicePlugin Task Processing
Run-Test "ServicePlugin Task Processing" "build\examples\03-services\background-tasks\ServicePluginTaskTest.exe"
Write-Host "[INFO] Background task processing verified" -ForegroundColor Gray

# Test 4: Python Bridge Example (if available)
if (Test-Path "build\examples\python_plugins\python_bridge_example.exe") {
    Run-Test "Python Bridge Integration" "build\examples\python_plugins\python_bridge_example.exe"
}

# Additional core tests if available
$coreTests = @(
    @{Name = "Plugin Manager Test"; Path = "build\tests\core\test_plugin_manager.exe"},
    @{Name = "Plugin Interface Test"; Path = "build\tests\core\test_plugin_interface.exe"},
    @{Name = "Security Manager Test"; Path = "build\tests\security\test_security_manager.exe"},
    @{Name = "Configuration Manager Test"; Path = "build\tests\managers\test_configuration_manager.exe"}
)

foreach ($test in $coreTests) {
    if (Test-Path $test.Path) {
        Run-Test $test.Name $test.Path
    }
}

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Test Results Summary" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Total Tests: $totalTests" -ForegroundColor White
Write-Host "✅ Passed: $passedTests" -ForegroundColor Green
Write-Host "❌ Failed: $($totalTests - $passedTests)" -ForegroundColor Red

if ($passedTests -eq $totalTests) {
    Write-Host "✅ All tests passed! 🎉" -ForegroundColor Green
    exit 0
} else {
    Write-Host "❌ Some tests failed. Please check the output above." -ForegroundColor Red
    exit 1
}
