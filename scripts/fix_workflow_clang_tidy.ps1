# Script to apply common clang-tidy fixes to workflow files
param(
    [string]$FilePath
)

$content = Get-Content -Path $FilePath -Raw

# Fix 1: Replace old logging category names with new snake_case names
$baseName = (Get-Item $FilePath).BaseName
$oldLogName = "workflow" + ($baseName -replace '_', '' | ForEach-Object { $_.Substring(0,1).ToUpper() + $_.Substring(1) }) + "Log"
$newLogName = "workflow_" + $baseName + "_log"

Write-Host "Processing $FilePath"
Write-Host "  Old log name: $oldLogName"
Write-Host "  New log name: $newLogName"

# Replace logging category references
$content = $content -replace $oldLogName, $newLogName

# Save the file
Set-Content -Path $FilePath -Value $content -NoNewline

Write-Host "  Fixed logging category references"
