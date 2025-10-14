/**
 * @file state_persistence.cpp
 * @brief Implementation of workflow state persistence and serialization
 * @version 3.1.0
 */

#include "qtplugin/workflow/state_persistence.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QUuid>

namespace {
Q_LOGGING_CATEGORY(workflow_state_persistence_log,
                   "qtplugin.workflow.state_persistence")
}  // namespace

namespace qtplugin::workflow::state {

// === WorkflowStepState Implementation ===

QJsonObject WorkflowStepState::to_json() const {
    QJsonObject json;
    json["step_id"] = step_id;
    json["state"] = static_cast<int>(state);
    json["input_data"] = input_data;
    json["output_data"] = output_data;
    json["error_data"] = error_data;
    json["start_time"] = start_time.toString(Qt::ISODate);
    json["end_time"] = end_time.toString(Qt::ISODate);
    json["retry_count"] = retry_count;
    json["metadata"] = metadata;
    return json;
}

qtplugin::expected<WorkflowStepState, PluginError> WorkflowStepState::from_json(
    const QJsonObject& json) {
    WorkflowStepState state;

    if (!json.contains("step_id") || !json["step_id"].isString()) {
        return make_error<WorkflowStepState>(PluginErrorCode::InvalidFormat,
                                             "Missing or invalid step_id");
    }
    state.step_id = json["step_id"].toString();

    if (json.contains("state") && json["state"].isDouble()) {
        state.state = static_cast<StepExecutionState>(json["state"].toInt());
    }

    if (json.contains("input_data") && json["input_data"].isObject()) {
        state.input_data = json["input_data"].toObject();
    }

    if (json.contains("output_data") && json["output_data"].isObject()) {
        state.output_data = json["output_data"].toObject();
    }

    if (json.contains("error_data") && json["error_data"].isObject()) {
        state.error_data = json["error_data"].toObject();
    }

    if (json.contains("start_time") && json["start_time"].isString()) {
        state.start_time =
            QDateTime::fromString(json["start_time"].toString(), Qt::ISODate);
    }

    if (json.contains("end_time") && json["end_time"].isString()) {
        state.end_time =
            QDateTime::fromString(json["end_time"].toString(), Qt::ISODate);
    }

    if (json.contains("retry_count") && json["retry_count"].isDouble()) {
        state.retry_count = json["retry_count"].toInt();
    }

    if (json.contains("metadata") && json["metadata"].isObject()) {
        state.metadata = json["metadata"].toObject();
    }

    return state;
}

// === WorkflowExecutionContext Implementation ===

QJsonObject WorkflowExecutionContext::to_json() const {
    QJsonObject json;
    json["execution_id"] = execution_id;
    json["workflow_id"] = workflow_id;
    json["workflow_name"] = workflow_name;
    json["state"] = static_cast<int>(state);
    json["initial_data"] = initial_data;
    json["final_result"] = final_result;
    json["error_data"] = error_data;
    json["start_time"] = start_time.toString(Qt::ISODate);
    json["end_time"] = end_time.toString(Qt::ISODate);
    json["current_step_id"] = current_step_id;
    json["execution_metadata"] = execution_metadata;
    json["is_transactional"] = is_transactional;
    json["transaction_id"] = transaction_id;
    json["is_composite"] = is_composite;
    json["composition_id"] = composition_id;

    // Serialize step states
    QJsonObject step_states_json;
    for (const auto& [step_id, step_state] : step_states) {
        step_states_json[step_id] = step_state.to_json();
    }
    json["step_states"] = step_states_json;

    return json;
}

qtplugin::expected<WorkflowExecutionContext, PluginError>
WorkflowExecutionContext::from_json(const QJsonObject& json) {
    WorkflowExecutionContext context;

    // Required fields
    if (!json.contains("execution_id") || !json["execution_id"].isString()) {
        return make_error<WorkflowExecutionContext>(
            PluginErrorCode::InvalidFormat, "Missing or invalid execution_id");
    }
    context.execution_id = json["execution_id"].toString();

    if (!json.contains("workflow_id") || !json["workflow_id"].isString()) {
        return make_error<WorkflowExecutionContext>(
            PluginErrorCode::InvalidFormat, "Missing or invalid workflow_id");
    }
    context.workflow_id = json["workflow_id"].toString();

    // Optional fields
    if (json.contains("workflow_name") && json["workflow_name"].isString()) {
        context.workflow_name = json["workflow_name"].toString();
    }

    if (json.contains("state") && json["state"].isDouble()) {
        context.state =
            static_cast<WorkflowExecutionState>(json["state"].toInt());
    }

    if (json.contains("initial_data") && json["initial_data"].isObject()) {
        context.initial_data = json["initial_data"].toObject();
    }

    if (json.contains("final_result") && json["final_result"].isObject()) {
        context.final_result = json["final_result"].toObject();
    }

    if (json.contains("error_data") && json["error_data"].isObject()) {
        context.error_data = json["error_data"].toObject();
    }

    if (json.contains("start_time") && json["start_time"].isString()) {
        context.start_time =
            QDateTime::fromString(json["start_time"].toString(), Qt::ISODate);
    }

    if (json.contains("end_time") && json["end_time"].isString()) {
        context.end_time =
            QDateTime::fromString(json["end_time"].toString(), Qt::ISODate);
    }

    if (json.contains("current_step_id") &&
        json["current_step_id"].isString()) {
        context.current_step_id = json["current_step_id"].toString();
    }

    if (json.contains("execution_metadata") &&
        json["execution_metadata"].isObject()) {
        context.execution_metadata = json["execution_metadata"].toObject();
    }

    if (json.contains("is_transactional") &&
        json["is_transactional"].isBool()) {
        context.is_transactional = json["is_transactional"].toBool();
    }

    if (json.contains("transaction_id") && json["transaction_id"].isString()) {
        context.transaction_id = json["transaction_id"].toString();
    }

    if (json.contains("is_composite") && json["is_composite"].isBool()) {
        context.is_composite = json["is_composite"].toBool();
    }

    if (json.contains("composition_id") && json["composition_id"].isString()) {
        context.composition_id = json["composition_id"].toString();
    }

    // Deserialize step states
    if (json.contains("step_states") && json["step_states"].isObject()) {
        QJsonObject step_states_json = json["step_states"].toObject();
        for (auto it = step_states_json.begin(); it != step_states_json.end();
             ++it) {
            const QString& step_id = it.key();
            const QJsonObject& step_json = it.value().toObject();

            auto step_state_result = WorkflowStepState::from_json(step_json);
            if (!step_state_result) {
                return qtplugin::unexpected<PluginError>(
                    step_state_result.error());
            }

            context.step_states[step_id] = step_state_result.value();
        }
    }

    return context;
}

void WorkflowExecutionContext::update_step_state(
    const QString& step_id, const WorkflowStepState& state) {
    step_states[step_id] = state;

    // Update current step if this step is running
    if (state.state == StepExecutionState::Running) {
        current_step_id = step_id;
    }

    qCDebug(workflow_state_persistence_log)
        << "Updated step state for execution:" << execution_id
        << "step:" << step_id << "state:" << static_cast<int>(state.state);
}

std::optional<WorkflowStepState> WorkflowExecutionContext::get_step_state(
    const QString& step_id) const {
    auto it = step_states.find(step_id);
    if (it != step_states.end()) {
        return it->second;
    }
    return std::nullopt;
}

double WorkflowExecutionContext::calculate_progress() const {
    if (step_states.empty()) {
        return 0.0;
    }

    size_t completed_steps = 0;
    for (const auto& [step_id, step_state] : step_states) {
        if (step_state.state == StepExecutionState::Completed ||
            step_state.state == StepExecutionState::Skipped) {
            completed_steps++;
        }
    }

    return (static_cast<double>(completed_steps) / step_states.size()) * 100.0;
}

// === WorkflowCheckpoint Implementation ===

QJsonObject WorkflowCheckpoint::to_json() const {
    QJsonObject json;
    json["checkpoint_id"] = checkpoint_id;
    json["execution_id"] = execution_id;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["context"] = context.to_json();
    json["checkpoint_metadata"] = checkpoint_metadata;
    return json;
}

qtplugin::expected<WorkflowCheckpoint, PluginError>
WorkflowCheckpoint::from_json(const QJsonObject& json) {
    WorkflowCheckpoint checkpoint;

    if (!json.contains("checkpoint_id") || !json["checkpoint_id"].isString()) {
        return make_error<WorkflowCheckpoint>(
            PluginErrorCode::InvalidFormat, "Missing or invalid checkpoint_id");
    }
    checkpoint.checkpoint_id = json["checkpoint_id"].toString();

    if (!json.contains("execution_id") || !json["execution_id"].isString()) {
        return make_error<WorkflowCheckpoint>(
            PluginErrorCode::InvalidFormat, "Missing or invalid execution_id");
    }
    checkpoint.execution_id = json["execution_id"].toString();

    if (json.contains("timestamp") && json["timestamp"].isString()) {
        checkpoint.timestamp =
            QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    }

    if (json.contains("context") && json["context"].isObject()) {
        auto context_result =
            WorkflowExecutionContext::from_json(json["context"].toObject());
        if (!context_result) {
            return qtplugin::unexpected<PluginError>(context_result.error());
        }
        checkpoint.context = context_result.value();
    }

    if (json.contains("checkpoint_metadata") &&
        json["checkpoint_metadata"].isObject()) {
        checkpoint.checkpoint_metadata = json["checkpoint_metadata"].toObject();
    }

    return checkpoint;
}

// === StatePersistenceConfig Implementation ===

QJsonObject StatePersistenceConfig::to_json() const {
    QJsonObject json;
    json["enabled"] = enabled;
    json["checkpoint_interval_ms"] =
        static_cast<int>(checkpoint_interval.count());
    json["max_checkpoints_per_workflow"] =
        static_cast<int>(max_checkpoints_per_workflow);
    json["storage_directory"] = storage_directory;
    json["compress_checkpoints"] = compress_checkpoints;
    json["encrypt_checkpoints"] = encrypt_checkpoints;
    return json;
}

qtplugin::expected<StatePersistenceConfig, PluginError>
StatePersistenceConfig::from_json(const QJsonObject& json) {
    StatePersistenceConfig config;

    if (json.contains("enabled") && json["enabled"].isBool()) {
        config.enabled = json["enabled"].toBool();
    }

    if (json.contains("checkpoint_interval_ms") &&
        json["checkpoint_interval_ms"].isDouble()) {
        config.checkpoint_interval =
            std::chrono::milliseconds(json["checkpoint_interval_ms"].toInt());
    }

    if (json.contains("max_checkpoints_per_workflow") &&
        json["max_checkpoints_per_workflow"].isDouble()) {
        config.max_checkpoints_per_workflow =
            static_cast<size_t>(json["max_checkpoints_per_workflow"].toInt());
    }

    if (json.contains("storage_directory") &&
        json["storage_directory"].isString()) {
        config.storage_directory = json["storage_directory"].toString();
    }

    if (json.contains("compress_checkpoints") &&
        json["compress_checkpoints"].isBool()) {
        config.compress_checkpoints = json["compress_checkpoints"].toBool();
    }

    if (json.contains("encrypt_checkpoints") &&
        json["encrypt_checkpoints"].isBool()) {
        config.encrypt_checkpoints = json["encrypt_checkpoints"].toBool();
    }

    return config;
}

// === FileWorkflowStateStorage Implementation ===

FileWorkflowStateStorage::FileWorkflowStateStorage(
    const QString& base_directory)
    : m_base_directory(base_directory) {
    qCDebug(workflow_state_persistence_log)
        << "Created file workflow state storage with base directory:"
        << base_directory;

    // Ensure base directory exists
    auto result = ensure_directory_exists(m_base_directory);
    if (!result) {
        qCWarning(workflow_state_persistence_log)
            << "Failed to create base directory:" << base_directory;
    }
}

qtplugin::expected<void, PluginError> FileWorkflowStateStorage::save_checkpoint(
    const WorkflowCheckpoint& checkpoint) {
    QString checkpoint_path = get_checkpoint_path(checkpoint.checkpoint_id);

    // Ensure directory exists
    QFileInfo file_info(checkpoint_path);
    auto dir_result = ensure_directory_exists(file_info.absolutePath());
    if (!dir_result) {
        return dir_result;
    }

    // Save checkpoint
    auto save_result = save_json_file(checkpoint_path, checkpoint.to_json());
    if (!save_result) {
        return save_result;
    }

    qCDebug(workflow_state_persistence_log)
        << "Saved checkpoint:" << checkpoint.checkpoint_id
        << "for execution:" << checkpoint.execution_id;

    return make_success();
}

qtplugin::expected<WorkflowCheckpoint, PluginError>
FileWorkflowStateStorage::load_checkpoint(const QString& checkpoint_id) {
    QString checkpoint_path = get_checkpoint_path(checkpoint_id);

    auto json_result = load_json_file(checkpoint_path);
    if (!json_result) {
        return qtplugin::unexpected<PluginError>(json_result.error());
    }

    auto checkpoint_result = WorkflowCheckpoint::from_json(json_result.value());
    if (!checkpoint_result) {
        return qtplugin::unexpected<PluginError>(checkpoint_result.error());
    }

    qCDebug(workflow_state_persistence_log)
        << "Loaded checkpoint:" << checkpoint_id;

    return checkpoint_result.value();
}

qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError>
FileWorkflowStateStorage::list_checkpoints(const QString& execution_id) {
    QString execution_dir = get_execution_directory(execution_id);
    QDir dir(execution_dir);

    if (!dir.exists()) {
        return std::vector<WorkflowCheckpoint>{};  // Return empty list if
                                                   // directory doesn't exist
    }

    QStringList checkpoint_files = dir.entryList(
        QStringList() << "checkpoint_*.json", QDir::Files, QDir::Name);
    std::vector<WorkflowCheckpoint> checkpoints;

    for (const QString& filename : checkpoint_files) {
        QString checkpoint_path = dir.absoluteFilePath(filename);

        auto json_result = load_json_file(checkpoint_path);
        if (!json_result) {
            qCWarning(workflow_state_persistence_log)
                << "Failed to load checkpoint file:" << checkpoint_path;
            continue;
        }

        auto checkpoint_result =
            WorkflowCheckpoint::from_json(json_result.value());
        if (!checkpoint_result) {
            qCWarning(workflow_state_persistence_log)
                << "Failed to parse checkpoint file:" << checkpoint_path;
            continue;
        }

        checkpoints.push_back(checkpoint_result.value());
    }

    qCDebug(workflow_state_persistence_log)
        << "Listed" << checkpoints.size()
        << "checkpoints for execution:" << execution_id;

    return checkpoints;
}

qtplugin::expected<void, PluginError>
FileWorkflowStateStorage::delete_checkpoint(const QString& checkpoint_id) {
    QString checkpoint_path = get_checkpoint_path(checkpoint_id);

    QFile file(checkpoint_path);
    if (!file.exists()) {
        return make_error<void>(
            PluginErrorCode::FileNotFound,
            "Checkpoint file not found: " + checkpoint_id.toStdString());
    }

    if (!file.remove()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to delete checkpoint file: " +
                                    file.errorString().toStdString());
    }

    qCDebug(workflow_state_persistence_log)
        << "Deleted checkpoint:" << checkpoint_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
FileWorkflowStateStorage::save_execution_context(
    const WorkflowExecutionContext& context) {
    QString context_path = get_execution_context_path(context.execution_id);

    // Ensure directory exists
    QFileInfo file_info(context_path);
    auto dir_result = ensure_directory_exists(file_info.absolutePath());
    if (!dir_result) {
        return dir_result;
    }

    // Save execution context
    auto save_result = save_json_file(context_path, context.to_json());
    if (!save_result) {
        return save_result;
    }

    qCDebug(workflow_state_persistence_log)
        << "Saved execution context:" << context.execution_id;

    return make_success();
}

qtplugin::expected<WorkflowExecutionContext, PluginError>
FileWorkflowStateStorage::load_execution_context(const QString& execution_id) {
    QString context_path = get_execution_context_path(execution_id);

    auto json_result = load_json_file(context_path);
    if (!json_result) {
        return qtplugin::unexpected<PluginError>(json_result.error());
    }

    auto context_result =
        WorkflowExecutionContext::from_json(json_result.value());
    if (!context_result) {
        return qtplugin::unexpected<PluginError>(context_result.error());
    }

    qCDebug(workflow_state_persistence_log)
        << "Loaded execution context:" << execution_id;

    return context_result.value();
}

qtplugin::expected<void, PluginError>
FileWorkflowStateStorage::delete_execution_context(
    const QString& execution_id) {
    QString execution_dir = get_execution_directory(execution_id);
    QDir dir(execution_dir);

    if (!dir.exists()) {
        return make_error<void>(
            PluginErrorCode::FileNotFound,
            "Execution directory not found: " + execution_id.toStdString());
    }

    // Remove all files in the execution directory
    if (!dir.removeRecursively()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to delete execution directory: " +
                                    execution_id.toStdString());
    }

    qCDebug(workflow_state_persistence_log)
        << "Deleted execution context and all checkpoints for:" << execution_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
FileWorkflowStateStorage::cleanup_old_checkpoints(std::chrono::hours max_age) {
    QDir base_dir(m_base_directory);
    if (!base_dir.exists()) {
        return make_success();  // Nothing to clean up
    }

    QDateTime cutoff_time = QDateTime::currentDateTime().addSecs(
        -static_cast<qint64>(max_age.count() * 3600));

    QStringList execution_dirs =
        base_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    size_t cleaned_count = 0;

    for (const QString& execution_dir : execution_dirs) {
        QString execution_path = base_dir.absoluteFilePath(execution_dir);
        QDir exec_dir(execution_path);

        // Check if execution context file is older than cutoff
        QString context_file = exec_dir.absoluteFilePath("context.json");
        QFileInfo context_info(context_file);

        if (context_info.exists() &&
            context_info.lastModified() < cutoff_time) {
            if (exec_dir.removeRecursively()) {
                cleaned_count++;
                qCDebug(workflow_state_persistence_log)
                    << "Cleaned up old execution:" << execution_dir;
            } else {
                qCWarning(workflow_state_persistence_log)
                    << "Failed to clean up execution:" << execution_dir;
            }
        }
    }

    qCDebug(workflow_state_persistence_log)
        << "Cleaned up" << cleaned_count << "old executions";

    return make_success();
}

// === Private Helper Methods ===

QString FileWorkflowStateStorage::get_checkpoint_path(
    const QString& checkpoint_id) const {
    // Extract execution ID from checkpoint ID (format: execution_id_timestamp)
    QStringList parts = checkpoint_id.split('_');
    if (parts.size() >= 2) {
        QString execution_id = parts[0];
        return QDir(get_execution_directory(execution_id))
            .absoluteFilePath(QString("checkpoint_%1.json").arg(checkpoint_id));
    }

    // Fallback: use checkpoint_id directly
    return QDir(m_base_directory)
        .absoluteFilePath(QString("checkpoint_%1.json").arg(checkpoint_id));
}

QString FileWorkflowStateStorage::get_execution_context_path(
    const QString& execution_id) const {
    return QDir(get_execution_directory(execution_id))
        .absoluteFilePath("context.json");
}

QString FileWorkflowStateStorage::get_execution_directory(
    const QString& execution_id) const {
    return QDir(m_base_directory).absoluteFilePath(execution_id);
}

qtplugin::expected<void, PluginError>
FileWorkflowStateStorage::ensure_directory_exists(const QString& path) const {
    QDir dir;
    if (!dir.mkpath(path)) {
        return make_error<void>(
            PluginErrorCode::FileSystemError,
            "Failed to create directory: " + path.toStdString());
    }
    return make_success();
}

qtplugin::expected<QJsonObject, PluginError>
FileWorkflowStateStorage::load_json_file(const QString& file_path) const {
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return make_error<QJsonObject>(
            PluginErrorCode::FileSystemError,
            "Failed to open file: " + file.errorString().toStdString());
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        return make_error<QJsonObject>(
            PluginErrorCode::InvalidFormat,
            "Failed to parse JSON: " + parse_error.errorString().toStdString());
    }

    if (!doc.isObject()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidFormat,
                                       "JSON document is not an object");
    }

    return doc.object();
}

qtplugin::expected<void, PluginError> FileWorkflowStateStorage::save_json_file(
    const QString& file_path, const QJsonObject& json) const {
    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to open file for writing: " +
                                    file.errorString().toStdString());
    }

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    if (file.write(data) != data.size()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to write complete data to file");
    }

    file.close();
    return make_success();
}

// === WorkflowCheckpointManager Implementation ===

WorkflowCheckpointManager::WorkflowCheckpointManager(
    std::unique_ptr<IWorkflowStateStorage> storage,
    const StatePersistenceConfig& config, QObject* parent)
    : QObject(parent), m_storage(std::move(storage)), m_config(config) {
    qCDebug(workflow_state_persistence_log)
        << "Created workflow checkpoint manager with config:"
        << "enabled:" << m_config.enabled
        << "interval:" << m_config.checkpoint_interval.count() << "ms";
}

void WorkflowCheckpointManager::set_config(
    const StatePersistenceConfig& config) {
    m_config = config;

    // Update existing timers if interval changed
    for (auto& [execution_id, timer] : m_checkpoint_timers) {
        if (timer && m_config.enabled) {
            timer->setInterval(
                static_cast<int>(m_config.checkpoint_interval.count()));
        }
    }

    qCDebug(workflow_state_persistence_log)
        << "Updated checkpoint manager config";
}

qtplugin::expected<QString, PluginError>
WorkflowCheckpointManager::create_checkpoint(
    const WorkflowExecutionContext& context, const QJsonObject& metadata) {
    if (!m_config.enabled) {
        return make_error<QString>(PluginErrorCode::InvalidConfiguration,
                                   "Checkpoint persistence is disabled");
    }

    QString checkpoint_id = generate_checkpoint_id(context.execution_id);

    WorkflowCheckpoint checkpoint;
    checkpoint.checkpoint_id = checkpoint_id;
    checkpoint.execution_id = context.execution_id;
    checkpoint.timestamp = QDateTime::currentDateTime();
    checkpoint.context = context;
    checkpoint.checkpoint_metadata = metadata;

    auto save_result = m_storage->save_checkpoint(checkpoint);
    if (!save_result) {
        emit checkpoint_failed(
            context.execution_id,
            QString::fromStdString(save_result.error().message));
        return qtplugin::unexpected<PluginError>(save_result.error());
    }

    // Cleanup old checkpoints if we exceed the limit
    auto checkpoints_result = m_storage->list_checkpoints(context.execution_id);
    if (checkpoints_result && checkpoints_result.value().size() >
                                  m_config.max_checkpoints_per_workflow) {
        // Sort by timestamp and remove oldest
        auto checkpoints = checkpoints_result.value();
        std::sort(checkpoints.begin(), checkpoints.end(),
                  [](const WorkflowCheckpoint& a, const WorkflowCheckpoint& b) {
                      return a.timestamp < b.timestamp;
                  });

        // Remove oldest checkpoints
        size_t to_remove =
            checkpoints.size() - m_config.max_checkpoints_per_workflow;
        for (size_t i = 0; i < to_remove; ++i) {
            auto delete_result =
                m_storage->delete_checkpoint(checkpoints[i].checkpoint_id);
            if (!delete_result) {
                qCWarning(workflow_state_persistence_log)
                    << "Failed to delete old checkpoint:"
                    << checkpoints[i].checkpoint_id;
            }
        }
    }

    emit checkpoint_created(checkpoint_id, context.execution_id);

    qCDebug(workflow_state_persistence_log)
        << "Created checkpoint:" << checkpoint_id
        << "for execution:" << context.execution_id;

    return checkpoint_id;
}

qtplugin::expected<WorkflowCheckpoint, PluginError>
WorkflowCheckpointManager::load_checkpoint(const QString& checkpoint_id) {
    return m_storage->load_checkpoint(checkpoint_id);
}

qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError>
WorkflowCheckpointManager::list_checkpoints(const QString& execution_id) {
    return m_storage->list_checkpoints(execution_id);
}

qtplugin::expected<void, PluginError>
WorkflowCheckpointManager::delete_checkpoint(const QString& checkpoint_id) {
    return m_storage->delete_checkpoint(checkpoint_id);
}

qtplugin::expected<void, PluginError>
WorkflowCheckpointManager::save_execution_context(
    const WorkflowExecutionContext& context) {
    auto save_result = m_storage->save_execution_context(context);
    if (!save_result) {
        emit execution_context_failed(
            context.execution_id,
            QString::fromStdString(save_result.error().message));
        return save_result;
    }

    emit execution_context_saved(context.execution_id);
    return save_result;
}

qtplugin::expected<WorkflowExecutionContext, PluginError>
WorkflowCheckpointManager::load_execution_context(const QString& execution_id) {
    return m_storage->load_execution_context(execution_id);
}

qtplugin::expected<void, PluginError>
WorkflowCheckpointManager::delete_execution_context(
    const QString& execution_id) {
    // Stop automatic checkpointing for this execution
    stop_automatic_checkpointing(execution_id);

    return m_storage->delete_execution_context(execution_id);
}

void WorkflowCheckpointManager::start_automatic_checkpointing(
    const QString& execution_id) {
    if (!m_config.enabled) {
        qCDebug(workflow_state_persistence_log)
            << "Automatic checkpointing disabled, not starting for:"
            << execution_id;
        return;
    }

    // Stop existing timer if any
    stop_automatic_checkpointing(execution_id);

    auto timer = std::make_unique<QTimer>(this);
    timer->setInterval(static_cast<int>(m_config.checkpoint_interval.count()));
    timer->setSingleShot(false);

    // Connect timer to checkpoint creation
    connect(timer.get(), &QTimer::timeout, this, [this, execution_id]() {
        auto it = m_active_contexts.find(execution_id);
        if (it != m_active_contexts.end()) {
            auto checkpoint_result = create_checkpoint(it->second);
            if (!checkpoint_result) {
                qCWarning(workflow_state_persistence_log)
                    << "Automatic checkpoint failed for execution:"
                    << execution_id << "error:"
                    << QString::fromStdString(
                           checkpoint_result.error().message);
            }
        }
    });

    timer->start();
    m_checkpoint_timers[execution_id] = std::move(timer);

    qCDebug(workflow_state_persistence_log)
        << "Started automatic checkpointing for execution:" << execution_id
        << "interval:" << m_config.checkpoint_interval.count() << "ms";
}

void WorkflowCheckpointManager::stop_automatic_checkpointing(
    const QString& execution_id) {
    cleanup_checkpoint_timer(execution_id);
    m_active_contexts.erase(execution_id);

    qCDebug(workflow_state_persistence_log)
        << "Stopped automatic checkpointing for execution:" << execution_id;
}

void WorkflowCheckpointManager::update_execution_context(
    const WorkflowExecutionContext& context) {
    m_active_contexts[context.execution_id] = context;

    // Save execution context immediately
    auto save_result = save_execution_context(context);
    if (!save_result) {
        qCWarning(workflow_state_persistence_log)
            << "Failed to save execution context:" << context.execution_id;
    }
}

qtplugin::expected<void, PluginError>
WorkflowCheckpointManager::cleanup_old_checkpoints(std::chrono::hours max_age) {
    return m_storage->cleanup_old_checkpoints(max_age);
}

void WorkflowCheckpointManager::on_checkpoint_timer() {
    // This slot is not used as we use lambda connections for per-execution
    // timers
}

QString WorkflowCheckpointManager::generate_checkpoint_id(
    const QString& execution_id) const {
    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    return QString("%1_%2").arg(execution_id, timestamp);
}

void WorkflowCheckpointManager::cleanup_checkpoint_timer(
    const QString& execution_id) {
    auto it = m_checkpoint_timers.find(execution_id);
    if (it != m_checkpoint_timers.end()) {
        if (it->second) {
            it->second->stop();
        }
        m_checkpoint_timers.erase(it);
    }
}

// === WorkflowRecoveryOptions Implementation ===

QJsonObject WorkflowRecoveryOptions::to_json() const {
    QJsonObject json;
    json["strategy"] = static_cast<int>(strategy);
    json["specific_checkpoint_id"] = specific_checkpoint_id;
    json["validate_checkpoint"] = validate_checkpoint;
    json["resume_execution"] = resume_execution;
    json["recovery_metadata"] = recovery_metadata;
    return json;
}

qtplugin::expected<WorkflowRecoveryOptions, PluginError>
WorkflowRecoveryOptions::from_json(const QJsonObject& json) {
    WorkflowRecoveryOptions options;

    if (json.contains("strategy") && json["strategy"].isDouble()) {
        options.strategy =
            static_cast<RecoveryStrategy>(json["strategy"].toInt());
    }

    if (json.contains("specific_checkpoint_id") &&
        json["specific_checkpoint_id"].isString()) {
        options.specific_checkpoint_id =
            json["specific_checkpoint_id"].toString();
    }

    if (json.contains("validate_checkpoint") &&
        json["validate_checkpoint"].isBool()) {
        options.validate_checkpoint = json["validate_checkpoint"].toBool();
    }

    if (json.contains("resume_execution") &&
        json["resume_execution"].isBool()) {
        options.resume_execution = json["resume_execution"].toBool();
    }

    if (json.contains("recovery_metadata") &&
        json["recovery_metadata"].isObject()) {
        options.recovery_metadata = json["recovery_metadata"].toObject();
    }

    return options;
}

// === WorkflowRecoveryResult Implementation ===

QJsonObject WorkflowRecoveryResult::to_json() const {
    QJsonObject json;
    json["success"] = success;
    json["execution_id"] = execution_id;
    json["checkpoint_id"] = checkpoint_id;
    json["restored_context"] = restored_context.to_json();
    json["recovery_metadata"] = recovery_metadata;
    json["error_message"] = error_message;
    return json;
}

qtplugin::expected<WorkflowRecoveryResult, PluginError>
WorkflowRecoveryResult::from_json(const QJsonObject& json) {
    WorkflowRecoveryResult result;

    if (json.contains("success") && json["success"].isBool()) {
        result.success = json["success"].toBool();
    }

    if (json.contains("execution_id") && json["execution_id"].isString()) {
        result.execution_id = json["execution_id"].toString();
    }

    if (json.contains("checkpoint_id") && json["checkpoint_id"].isString()) {
        result.checkpoint_id = json["checkpoint_id"].toString();
    }

    if (json.contains("restored_context") &&
        json["restored_context"].isObject()) {
        auto context_result = WorkflowExecutionContext::from_json(
            json["restored_context"].toObject());
        if (!context_result) {
            return qtplugin::unexpected<PluginError>(context_result.error());
        }
        result.restored_context = context_result.value();
    }

    if (json.contains("recovery_metadata") &&
        json["recovery_metadata"].isObject()) {
        result.recovery_metadata = json["recovery_metadata"].toObject();
    }

    if (json.contains("error_message") && json["error_message"].isString()) {
        result.error_message = json["error_message"].toString();
    }

    return result;
}

// === WorkflowRecoveryManager Implementation ===

WorkflowRecoveryManager::WorkflowRecoveryManager(
    WorkflowCheckpointManager* checkpoint_manager, QObject* parent)
    : QObject(parent), m_checkpoint_manager(checkpoint_manager) {
    qCDebug(workflow_state_persistence_log)
        << "Created workflow recovery manager";
}

qtplugin::expected<WorkflowRecoveryResult, PluginError>
WorkflowRecoveryManager::recover_workflow(
    const QString& execution_id, const WorkflowRecoveryOptions& options) {
    qCDebug(workflow_state_persistence_log)
        << "Starting workflow recovery for execution:" << execution_id
        << "strategy:" << static_cast<int>(options.strategy);

    emit recovery_started(execution_id, "");

    WorkflowRecoveryResult result;
    result.execution_id = execution_id;

    try {
        // Select checkpoint based on strategy
        auto checkpoint_result =
            select_checkpoint_by_strategy(execution_id, options);
        if (!checkpoint_result) {
            result.error_message =
                QString::fromStdString(checkpoint_result.error().message);
            emit recovery_failed(execution_id, result.error_message);
            return result;
        }

        auto checkpoint = checkpoint_result.value();
        result.checkpoint_id = checkpoint.checkpoint_id;

        // Validate checkpoint if requested
        if (options.validate_checkpoint) {
            auto validation_result =
                validate_checkpoint_for_recovery(checkpoint);
            if (!validation_result || !validation_result.value()) {
                result.error_message = "Checkpoint validation failed";
                emit recovery_failed(execution_id, result.error_message);
                return result;
            }
        }

        // Prepare recovery context
        auto context_result = prepare_recovery_context(checkpoint, options);
        if (!context_result) {
            result.error_message =
                QString::fromStdString(context_result.error().message);
            emit recovery_failed(execution_id, result.error_message);
            return result;
        }

        result.restored_context = context_result.value();
        result.success = true;
        result.recovery_metadata = options.recovery_metadata;
        result.recovery_metadata["recovery_timestamp"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        result.recovery_metadata["original_checkpoint_timestamp"] =
            checkpoint.timestamp.toString(Qt::ISODate);

        qCDebug(workflow_state_persistence_log)
            << "Workflow recovery completed successfully for execution:"
            << execution_id << "from checkpoint:" << checkpoint.checkpoint_id;

        emit recovery_completed(result);
        return result;

    } catch (const std::exception& e) {
        result.error_message =
            QString("Recovery failed with exception: %1").arg(e.what());
        emit recovery_failed(execution_id, result.error_message);
        return result;
    }
}

qtplugin::expected<WorkflowRecoveryResult, PluginError>
WorkflowRecoveryManager::recover_from_checkpoint(
    const QString& checkpoint_id, const WorkflowRecoveryOptions& options) {
    qCDebug(workflow_state_persistence_log)
        << "Starting workflow recovery from checkpoint:" << checkpoint_id;

    // Load the specific checkpoint
    auto checkpoint_result =
        m_checkpoint_manager->load_checkpoint(checkpoint_id);
    if (!checkpoint_result) {
        return qtplugin::unexpected<PluginError>(checkpoint_result.error());
    }

    auto checkpoint = checkpoint_result.value();
    emit recovery_started(checkpoint.execution_id, checkpoint_id);

    // Create modified options for specific checkpoint recovery
    WorkflowRecoveryOptions modified_options = options;
    modified_options.strategy = RecoveryStrategy::RestoreFromSpecific;
    modified_options.specific_checkpoint_id = checkpoint_id;

    return recover_workflow(checkpoint.execution_id, modified_options);
}

qtplugin::expected<bool, PluginError>
WorkflowRecoveryManager::validate_checkpoint_for_recovery(
    const WorkflowCheckpoint& checkpoint) {
    // Basic validation checks
    if (checkpoint.checkpoint_id.isEmpty()) {
        return make_error<bool>(PluginErrorCode::InvalidParameters,
                                "Checkpoint ID is empty");
    }

    if (checkpoint.execution_id.isEmpty()) {
        return make_error<bool>(PluginErrorCode::InvalidParameters,
                                "Execution ID is empty");
    }

    if (checkpoint.context.execution_id != checkpoint.execution_id) {
        return make_error<bool>(PluginErrorCode::InvalidConfiguration,
                                "Execution ID mismatch in checkpoint context");
    }

    // Check if checkpoint is not too old (optional validation)
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-7);  // 7 days old
    if (checkpoint.timestamp < cutoff) {
        qCWarning(workflow_state_persistence_log)
            << "Checkpoint is older than 7 days:" << checkpoint.checkpoint_id;
        // Don't fail, just warn
    }

    // Validate execution context
    if (checkpoint.context.workflow_id.isEmpty()) {
        return make_error<bool>(PluginErrorCode::InvalidConfiguration,
                                "Workflow ID is empty in checkpoint context");
    }

    // Check if workflow is in a recoverable state
    if (checkpoint.context.state == WorkflowExecutionState::Completed) {
        return make_error<bool>(PluginErrorCode::InvalidConfiguration,
                                "Cannot recover from completed workflow");
    }

    qCDebug(workflow_state_persistence_log)
        << "Checkpoint validation passed for:" << checkpoint.checkpoint_id;
    return true;
}

qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError>
WorkflowRecoveryManager::find_recoverable_checkpoints(
    const QString& execution_id) {
    auto checkpoints_result =
        m_checkpoint_manager->list_checkpoints(execution_id);
    if (!checkpoints_result) {
        return qtplugin::unexpected<PluginError>(checkpoints_result.error());
    }

    std::vector<WorkflowCheckpoint> recoverable_checkpoints;

    for (const auto& checkpoint : checkpoints_result.value()) {
        auto validation_result = validate_checkpoint_for_recovery(checkpoint);
        if (validation_result && validation_result.value()) {
            recoverable_checkpoints.push_back(checkpoint);
        }
    }

    // Sort by timestamp (newest first)
    std::sort(recoverable_checkpoints.begin(), recoverable_checkpoints.end(),
              [](const WorkflowCheckpoint& a, const WorkflowCheckpoint& b) {
                  return a.timestamp > b.timestamp;
              });

    qCDebug(workflow_state_persistence_log)
        << "Found" << recoverable_checkpoints.size()
        << "recoverable checkpoints for execution:" << execution_id;

    return recoverable_checkpoints;
}

qtplugin::expected<WorkflowCheckpoint, PluginError>
WorkflowRecoveryManager::find_best_checkpoint(const QString& execution_id) {
    auto checkpoints_result = find_recoverable_checkpoints(execution_id);
    if (!checkpoints_result) {
        return qtplugin::unexpected<PluginError>(checkpoints_result.error());
    }

    auto checkpoints = checkpoints_result.value();
    if (checkpoints.empty()) {
        return make_error<WorkflowCheckpoint>(
            PluginErrorCode::NotFound, "No recoverable checkpoints found");
    }

    // Find the best checkpoint (latest with successful steps)
    for (const auto& checkpoint : checkpoints) {
        // Prefer checkpoints where workflow is running or suspended
        if (checkpoint.context.state == WorkflowExecutionState::Running ||
            checkpoint.context.state == WorkflowExecutionState::Suspended) {
            // Check if there are completed steps
            bool has_completed_steps = false;
            for (const auto& [step_id, step_state] :
                 checkpoint.context.step_states) {
                if (step_state.state == StepExecutionState::Completed) {
                    has_completed_steps = true;
                    break;
                }
            }

            if (has_completed_steps) {
                qCDebug(workflow_state_persistence_log)
                    << "Selected best checkpoint:" << checkpoint.checkpoint_id
                    << "for execution:" << execution_id;
                return checkpoint;
            }
        }
    }

    // Fallback to latest checkpoint
    qCDebug(workflow_state_persistence_log)
        << "Using latest checkpoint as best:" << checkpoints[0].checkpoint_id
        << "for execution:" << execution_id;
    return checkpoints[0];
}

qtplugin::expected<WorkflowExecutionContext, PluginError>
WorkflowRecoveryManager::prepare_recovery_context(
    const WorkflowCheckpoint& checkpoint,
    const WorkflowRecoveryOptions& options) {
    WorkflowExecutionContext recovery_context = checkpoint.context;

    // Update recovery metadata
    recovery_context.execution_metadata["recovery_checkpoint_id"] =
        checkpoint.checkpoint_id;
    recovery_context.execution_metadata["recovery_timestamp"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);
    recovery_context.execution_metadata["recovery_strategy"] =
        static_cast<int>(options.strategy);

    // Merge recovery options metadata
    for (auto it = options.recovery_metadata.begin();
         it != options.recovery_metadata.end(); ++it) {
        recovery_context
            .execution_metadata[QString("recovery_%1").arg(it.key())] =
            it.value();
    }

    // Reset any failed steps if requested
    if (options.recovery_metadata.contains("reset_failed_steps") &&
        options.recovery_metadata["reset_failed_steps"].toBool()) {
        for (auto& [step_id, step_state] : recovery_context.step_states) {
            if (step_state.state == StepExecutionState::Failed) {
                step_state.state = StepExecutionState::Pending;
                step_state.error_data = QJsonObject{};
                step_state.retry_count = 0;
                step_state.end_time = QDateTime{};
            }
        }
    }

    // Update workflow state for recovery
    if (recovery_context.state == WorkflowExecutionState::Failed ||
        recovery_context.state == WorkflowExecutionState::Cancelled) {
        recovery_context.state = WorkflowExecutionState::Suspended;
    }

    qCDebug(workflow_state_persistence_log)
        << "Prepared recovery context for execution:"
        << recovery_context.execution_id
        << "from checkpoint:" << checkpoint.checkpoint_id;

    return recovery_context;
}

qtplugin::expected<WorkflowCheckpoint, PluginError>
WorkflowRecoveryManager::select_checkpoint_by_strategy(
    const QString& execution_id, const WorkflowRecoveryOptions& options) {
    switch (options.strategy) {
        case RecoveryStrategy::RestoreFromLatest: {
            auto checkpoints_result =
                find_recoverable_checkpoints(execution_id);
            if (!checkpoints_result) {
                return qtplugin::unexpected<PluginError>(
                    checkpoints_result.error());
            }

            auto checkpoints = checkpoints_result.value();
            if (checkpoints.empty()) {
                return make_error<WorkflowCheckpoint>(
                    PluginErrorCode::NotFound,
                    "No recoverable checkpoints found");
            }

            return checkpoints[0];  // Already sorted by timestamp (newest
                                    // first)
        }

        case RecoveryStrategy::RestoreFromSpecific: {
            if (options.specific_checkpoint_id.isEmpty()) {
                return make_error<WorkflowCheckpoint>(
                    PluginErrorCode::InvalidParameters,
                    "Specific checkpoint ID required for RestoreFromSpecific "
                    "strategy");
            }

            return m_checkpoint_manager->load_checkpoint(
                options.specific_checkpoint_id);
        }

        case RecoveryStrategy::RestoreFromBest: {
            return find_best_checkpoint(execution_id);
        }

        case RecoveryStrategy::RestartFromBeginning: {
            // For restart strategy, we need to create a fresh context
            // This would typically involve loading the original workflow
            // definition
            return make_error<WorkflowCheckpoint>(
                PluginErrorCode::NotImplemented,
                "RestartFromBeginning strategy not yet implemented");
        }

        default:
            return make_error<WorkflowCheckpoint>(
                PluginErrorCode::InvalidParameters,
                "Unknown recovery strategy");
    }
}

bool WorkflowRecoveryManager::is_checkpoint_valid_for_recovery(
    const WorkflowCheckpoint& checkpoint) {
    auto validation_result = validate_checkpoint_for_recovery(checkpoint);
    return validation_result && validation_result.value();
}

WorkflowExecutionContext WorkflowRecoveryManager::create_recovery_context(
    const WorkflowCheckpoint& checkpoint,
    const WorkflowRecoveryOptions& options) {
    auto context_result = prepare_recovery_context(checkpoint, options);
    if (context_result) {
        return context_result.value();
    }

    // Fallback to original context if preparation fails
    return checkpoint.context;
}

// === WorkflowStatePersistenceConfigManager Implementation ===

WorkflowStatePersistenceConfigManager::WorkflowStatePersistenceConfigManager(
    QObject* parent)
    : QObject(parent), m_config(create_default_config()) {
    qCDebug(workflow_state_persistence_log)
        << "Created workflow state persistence config manager";

    // Try to load existing configuration
    auto load_result = load_config();
    if (!load_result) {
        qCWarning(workflow_state_persistence_log)
            << "Failed to load workflow state persistence config, using "
               "defaults";
    }
}

qtplugin::expected<void, PluginError>
WorkflowStatePersistenceConfigManager::load_config() {
    // Load configuration from QtForge configuration system
    // We'll use the Global scope for workflow state persistence settings

    // For now, we'll use a simple approach - in a real implementation,
    // this would integrate with ConfigurationManager

    // Try to load from a default configuration file
    QString config_dir =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString config_file =
        QDir(config_dir).absoluteFilePath("workflow_state_persistence.json");

    QFile file(config_file);
    if (!file.exists()) {
        // No existing config, use defaults
        qCDebug(workflow_state_persistence_log)
            << "No existing config file, using defaults";
        emit config_loaded();
        return make_success();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return make_error<void>(
            PluginErrorCode::FileSystemError,
            "Failed to open config file: " + file.errorString().toStdString());
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        return make_error<void>(PluginErrorCode::InvalidFormat,
                                "Failed to parse config JSON: " +
                                    parse_error.errorString().toStdString());
    }

    if (!doc.isObject()) {
        return make_error<void>(PluginErrorCode::InvalidFormat,
                                "Config file is not a JSON object");
    }

    auto config_result = StatePersistenceConfig::from_json(doc.object());
    if (!config_result) {
        return qtplugin::unexpected<PluginError>(config_result.error());
    }

    StatePersistenceConfig old_config = m_config;
    m_config = config_result.value();

    // Validate the loaded configuration
    auto validation_result = validate_config();
    if (!validation_result) {
        // Revert to old config if validation fails
        m_config = old_config;
        return validation_result;
    }

    qCDebug(workflow_state_persistence_log)
        << "Loaded workflow state persistence config from:" << config_file;
    emit config_loaded();
    emit config_changed(m_config);

    return make_success();
}

qtplugin::expected<void, PluginError>
WorkflowStatePersistenceConfigManager::save_config() {
    QString config_dir =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir;
    if (!dir.mkpath(config_dir)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to create config directory");
    }

    QString config_file =
        QDir(config_dir).absoluteFilePath("workflow_state_persistence.json");

    QFile file(config_file);
    if (!file.open(QIODevice::WriteOnly)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to open config file for writing: " +
                                    file.errorString().toStdString());
    }

    QJsonDocument doc(m_config.to_json());
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    if (file.write(data) != data.size()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to write complete config data");
    }

    file.close();

    qCDebug(workflow_state_persistence_log)
        << "Saved workflow state persistence config to:" << config_file;
    emit config_saved();

    return make_success();
}

qtplugin::expected<void, PluginError>
WorkflowStatePersistenceConfigManager::reset_to_defaults() {
    StatePersistenceConfig old_config = m_config;
    m_config = create_default_config();

    auto save_result = save_config();
    if (!save_result) {
        // Revert if save fails
        m_config = old_config;
        return save_result;
    }

    qCDebug(workflow_state_persistence_log)
        << "Reset workflow state persistence config to defaults";
    emit config_changed(m_config);

    return make_success();
}

void WorkflowStatePersistenceConfigManager::set_config(
    const StatePersistenceConfig& config) {
    if (m_config.enabled != config.enabled ||
        m_config.checkpoint_interval != config.checkpoint_interval ||
        m_config.max_checkpoints_per_workflow !=
            config.max_checkpoints_per_workflow ||
        m_config.storage_directory != config.storage_directory ||
        m_config.compress_checkpoints != config.compress_checkpoints ||
        m_config.encrypt_checkpoints != config.encrypt_checkpoints) {
        m_config = config;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Updated workflow state persistence config";
    }
}

void WorkflowStatePersistenceConfigManager::set_enabled(bool enabled) {
    if (m_config.enabled != enabled) {
        m_config.enabled = enabled;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set workflow state persistence enabled:" << enabled;
    }
}

void WorkflowStatePersistenceConfigManager::set_checkpoint_interval(
    std::chrono::milliseconds interval) {
    if (m_config.checkpoint_interval != interval) {
        m_config.checkpoint_interval = interval;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set checkpoint interval:" << interval.count() << "ms";
    }
}

void WorkflowStatePersistenceConfigManager::set_max_checkpoints_per_workflow(
    size_t max_checkpoints) {
    if (m_config.max_checkpoints_per_workflow != max_checkpoints) {
        m_config.max_checkpoints_per_workflow = max_checkpoints;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set max checkpoints per workflow:" << max_checkpoints;
    }
}

void WorkflowStatePersistenceConfigManager::set_storage_directory(
    const QString& directory) {
    if (m_config.storage_directory != directory) {
        m_config.storage_directory = directory;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set storage directory:" << directory;
    }
}

void WorkflowStatePersistenceConfigManager::set_compress_checkpoints(
    bool compress) {
    if (m_config.compress_checkpoints != compress) {
        m_config.compress_checkpoints = compress;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set compress checkpoints:" << compress;
    }
}

void WorkflowStatePersistenceConfigManager::set_encrypt_checkpoints(
    bool encrypt) {
    if (m_config.encrypt_checkpoints != encrypt) {
        m_config.encrypt_checkpoints = encrypt;
        emit config_changed(m_config);

        qCDebug(workflow_state_persistence_log)
            << "Set encrypt checkpoints:" << encrypt;
    }
}

qtplugin::expected<void, PluginError>
WorkflowStatePersistenceConfigManager::validate_config() const {
    // Validate checkpoint interval
    if (m_config.checkpoint_interval.count() < 1000) {  // Less than 1 second
        return make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Checkpoint interval must be at least 1 second");
    }

    if (m_config.checkpoint_interval.count() > 3600000) {  // More than 1 hour
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Checkpoint interval must be less than 1 hour");
    }

    // Validate max checkpoints
    if (m_config.max_checkpoints_per_workflow == 0) {
        return make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Max checkpoints per workflow must be at least 1");
    }

    if (m_config.max_checkpoints_per_workflow > 1000) {
        return make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Max checkpoints per workflow must be less than 1000");
    }

    // Validate storage directory
    if (m_config.storage_directory.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Storage directory cannot be empty");
    }

    // Ensure storage directory exists or can be created
    auto dir_result = ensure_storage_directory();
    if (!dir_result) {
        return dir_result;
    }

    return make_success();
}

StatePersistenceConfig
WorkflowStatePersistenceConfigManager::create_default_config() {
    StatePersistenceConfig config;
    config.enabled = true;
    config.checkpoint_interval = std::chrono::seconds(30);
    config.max_checkpoints_per_workflow = 10;

    // Use a subdirectory in the app data location
    QString app_data =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    config.storage_directory =
        QDir(app_data).absoluteFilePath("workflow_state");

    config.compress_checkpoints = false;
    config.encrypt_checkpoints = false;

    return config;
}

qtplugin::expected<void, PluginError>
WorkflowStatePersistenceConfigManager::ensure_storage_directory() const {
    QDir dir;
    if (!dir.mkpath(m_config.storage_directory)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to create storage directory: " +
                                    m_config.storage_directory.toStdString());
    }

    // Check if directory is writable
    QFileInfo dir_info(m_config.storage_directory);
    if (!dir_info.isWritable()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Storage directory is not writable: " +
                                    m_config.storage_directory.toStdString());
    }

    return make_success();
}

}  // namespace qtplugin::workflow::state
