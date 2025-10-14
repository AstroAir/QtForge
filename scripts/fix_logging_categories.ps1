# Fix logging category declarations in workflow files

$files = @(
    "src/workflow/orchestration.cpp",
    "src/workflow/progress_message_bus.cpp",
    "src/workflow/progress_monitoring.cpp",
    "src/workflow/progress_tracking.cpp",
    "src/workflow/rollback_manager.cpp",
    "src/workflow/state_persistence.cpp",
    "src/workflow/transaction_error_handler.cpp",
    "src/workflow/workflow_manager.cpp",
    "src/workflow/workflow_validator.cpp"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "Processing $file..."

        $content = Get-Content -Path $file -Raw

        # Pattern to match Q_LOGGING_CATEGORY at global scope
        $pattern = '(#include[^\n]+\n(?:#include[^\n]+\n)*)\n(Q_LOGGING_CATEGORY\([^)]+\))'

        # Replacement with anonymous namespace
        $replacement = '$1' + "`n`nnamespace {`n" + '$2' + "`n}  // namespace"

        $content = $content -replace $pattern, $replacement

        # Save the file
        Set-Content -Path $file -Value $content -NoNewline
        Write-Host "  Done!"
    } else {
        Write-Host "File not found: $file"
    }
}

Write-Host "`nAll files processed!"
