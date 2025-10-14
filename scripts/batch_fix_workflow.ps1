# Batch fix common clang-tidy issues in workflow files

$files = @(
    "src/workflow/orchestration.cpp",
    "src/workflow/progress_message_bus.cpp",
    "src/workflow/progress_monitoring.cpp",
    "src/workflow/progress_tracking.cpp",
    "src/workflow/rollback_manager.cpp",
    "src/workflow/state_persistence.cpp",
    "src/workflow/transaction_error_handler.cpp",
    "src/workflow/transactions.cpp",
    "src/workflow/workflow_manager.cpp",
    "src/workflow/workflow_validator.cpp"
)

$logNameMap = @{
    "orchestration" = @("workflowOrchestrationLog", "workflow_orchestration_log")
    "progress_message_bus" = @("workflowProgressMessageBusLog", "workflow_progress_message_bus_log")
    "progress_monitoring" = @("workflowProgressMonitoringLog", "workflow_progress_monitoring_log")
    "progress_tracking" = @("workflowProgressTrackingLog", "workflow_progress_tracking_log")
    "rollback_manager" = @("workflowRollbackManagerLog", "workflow_rollback_manager_log")
    "state_persistence" = @("workflowStatePersistenceLog", "workflow_state_persistence_log")
    "transaction_error_handler" = @("workflowTransactionErrorHandlerLog", "workflow_transaction_error_handler_log")
    "transactions" = @("workflowTransactionsLog", "workflow_transactions_log")
    "workflow_manager" = @("workflowManagerLog", "workflow_manager_log")
    "workflow_validator" = @("workflowValidatorLog", "workflow_validator_log")
}

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "Processing $file..."

        $baseName = (Get-Item $file).BaseName
        $content = Get-Content -Path $file -Raw

        # Get the log name mapping
        if ($logNameMap.ContainsKey($baseName)) {
            $oldLog = $logNameMap[$baseName][0]
            $newLog = $logNameMap[$baseName][1]

            Write-Host "  Replacing $oldLog with $newLog"
            $content = $content -replace $oldLog, $newLog
        }

        # Save the file
        Set-Content -Path $file -Value $content -NoNewline
        Write-Host "  Done!"
    } else {
        Write-Host "File not found: $file"
    }
}

Write-Host "`nAll files processed!"
