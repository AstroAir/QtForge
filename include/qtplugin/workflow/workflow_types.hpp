/**
 * @file workflow_types.hpp
 * @brief Common types and forward declarations for the workflow module
 * @version 3.1.0
 * @author QtPlugin Development Team
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <memory>
#include <vector>
#include <chrono>

// Forward declarations
namespace qtplugin::workflow {
    namespace composition {
        class PluginComposition;
        class CompositePlugin;
        class CompositionManager;
        enum class CompositionStrategy;
        enum class PluginRole;
        struct CompositionBinding;
    }
    
    namespace orchestration {
        class Workflow;
        class PluginOrchestrator;
        struct WorkflowStep;
        struct WorkflowContext;
        struct StepResult;
        enum class ExecutionMode;
        enum class StepStatus;
    }
    
    namespace transactions {
        class PluginTransactionManager;
        class TransactionContext;
        class ITransactionParticipant;
        struct TransactionOperation;
        enum class TransactionState;
        enum class IsolationLevel;
        enum class OperationType;
    }
    
    namespace integration {
        class WorkflowManager;
        class UnifiedWorkflow;
        class TransactionalComposition;
    }
}

// Common type aliases
namespace qtplugin::workflow {
    using WorkflowId = QString;
    using ExecutionId = QString;
    using TransactionId = QString;
    using CompositionId = QString;
    using PluginId = QString;
}
