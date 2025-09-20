/**
 * @file workflow.hpp
 * @brief Main unified workflow module header
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file provides the main entry point for the unified workflow module
 * that combines composition, orchestration, and transaction management
 * capabilities into a single, cohesive system.
 *
 * ## Overview
 *
 * The unified workflow module consolidates three previously separate components:
 * - **Composition**: Plugin composition and aggregation patterns
 * - **Orchestration**: Multi-plugin workflow execution and coordination
 * - **Transactions**: Atomic operations and consistency management
 *
 * ## Usage
 *
 * ### Basic Usage
 * ```cpp
 * #include "qtplugin/workflow/workflow.hpp"
 *
 * using namespace qtplugin::workflow;
 *
 * // Access individual managers
 * auto& workflow_manager = integration::WorkflowManager::instance();
 * auto& composition_manager = workflow_manager.composition_manager();
 * auto& orchestrator = workflow_manager.orchestrator();
 * auto& transaction_manager = workflow_manager.transaction_manager();
 * ```
 *
 * ### Unified Workflows
 * ```cpp
 * // Create a unified workflow that combines all capabilities
 * integration::UnifiedWorkflow workflow("my-workflow", "My Unified Workflow");
 * workflow.set_transactional(true)
 *         .set_isolation_level(transactions::IsolationLevel::ReadCommitted);
 *
 * auto result = workflow.execute(initial_data);
 * ```
 *
 * ## Namespaces
 *
 * - `qtplugin::workflow::composition` - Plugin composition functionality
 * - `qtplugin::workflow::orchestration` - Workflow orchestration functionality
 * - `qtplugin::workflow::transactions` - Transaction management functionality
 * - `qtplugin::workflow::integration` - Unified workflow capabilities
 *
 * ## Backward Compatibility
 *
 * The original namespaces are available as aliases:
 * - `qtplugin::composition` → `qtplugin::workflow::composition`
 * - `qtplugin::orchestration` → `qtplugin::workflow::orchestration`
 * - `qtplugin::transactions` → `qtplugin::workflow::transactions`
 */

#pragma once

// Include all workflow functionality
#include "workflow_types.hpp"
#include "composition.hpp"
#include "orchestration.hpp"
#include "transactions.hpp"
#include "integration.hpp"

namespace qtplugin::workflow {
    /**
     * @brief Main unified workflow functionality
     *
     * This namespace provides access to all workflow capabilities through
     * organized sub-namespaces:
     *
     * - composition: Plugin composition and aggregation
     * - orchestration: Multi-plugin workflow execution
     * - transactions: Atomic operations and consistency
     * - integration: Unified workflow capabilities
     */

    // Convenience aliases for commonly used types
    using WorkflowManager = integration::WorkflowManager;
    using UnifiedWorkflow = integration::UnifiedWorkflow;
    using TransactionalComposition = integration::TransactionalComposition;
}

// Backward compatibility aliases
namespace qtplugin {
    /**
     * @brief Backward compatibility alias for composition functionality
     * @deprecated Use qtplugin::workflow::composition instead
     */
    namespace composition = workflow::composition;

    /**
     * @brief Backward compatibility alias for orchestration functionality
     * @deprecated Use qtplugin::workflow::orchestration instead
     */
    namespace orchestration = workflow::orchestration;

    /**
     * @brief Backward compatibility alias for transactions functionality
     * @deprecated Use qtplugin::workflow::transactions instead
     */
    namespace transactions = workflow::transactions;
}
