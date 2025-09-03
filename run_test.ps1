# PowerShell wrapper script to run tests with proper environment
param(
    [Parameter(Mandatory=$true)]
    [string]$TestExecutable,
    
    [Parameter(Mandatory=$true)]
    [string]$WorkingDirectory
)

# Set up the environment
$env:PATH = "$WorkingDirectory;D:\Project\QtForge\build;D:\msys64\mingw64\bin;$env:PATH"

# Change to the working directory
Set-Location $WorkingDirectory

# Run the test executable
& $TestExecutable

# Exit with the same code as the test
exit $LASTEXITCODE
