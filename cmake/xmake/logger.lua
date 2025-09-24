-- QtForge Comprehensive Error Handling and Logging Module
-- Robust error handling and logging throughout the modular build system
-- Version: 3.2.0

-- Module table
local logger = {}

-- Log levels
logger.levels = {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
}

-- Current log level
logger.current_level = logger.levels.INFO

-- Log configuration
logger.config = {
    enabled = true,
    file_logging = true,
    console_logging = true,
    log_file = "build/qtforge.log",
    max_file_size = 10 * 1024 * 1024, -- 10MB
    backup_count = 5,
    timestamp_format = "%Y-%m-%d %H:%M:%S",
    include_source = true
}

-- Color codes for console output
logger.colors = {
    TRACE = "\27[90m",   -- Dark gray
    DEBUG = "\27[36m",   -- Cyan
    INFO = "\27[32m",    -- Green
    WARN = "\27[33m",    -- Yellow
    ERROR = "\27[31m",   -- Red
    FATAL = "\27[35m",   -- Magenta
    RESET = "\27[0m"     -- Reset
}

-- Initialize logging system
function logger.init()
    if not logger.config.enabled then
        return
    end

    -- Create log directory
    local log_dir = path.directory(logger.config.log_file)
    if not os.isdir(log_dir) then
        os.mkdir(log_dir)
    end

    -- Rotate log files if needed
    logger.rotate_logs()

    -- Log initialization
    logger.info("QtForge logging system initialized")
    logger.info("Log file: " .. logger.config.log_file)
    logger.info("Log level: " .. logger.get_level_name(logger.current_level))
end

-- Set log level
function logger.set_level(level)
    if type(level) == "string" then
        level = logger.levels[level:upper()]
    end

    if level and level >= logger.levels.TRACE and level <= logger.levels.FATAL then
        logger.current_level = level
        logger.info("Log level set to: " .. logger.get_level_name(level))
    else
        logger.error("Invalid log level: " .. tostring(level))
    end
end

-- Get level name
function logger.get_level_name(level)
    for name, value in pairs(logger.levels) do
        if value == level then
            return name
        end
    end
    return "UNKNOWN"
end

-- Format log message
function logger.format_message(level, message, source_info)
    local timestamp = os.date(logger.config.timestamp_format)
    local level_name = logger.get_level_name(level)

    local formatted = string.format("[%s] [%s] %s", timestamp, level_name, message)

    if logger.config.include_source and source_info then
        formatted = formatted .. string.format(" (%s:%d)", source_info.file or "unknown", source_info.line or 0)
    end

    return formatted
end

-- Write to log file
function logger.write_to_file(message)
    if not logger.config.file_logging then
        return
    end

    local file = io.open(logger.config.log_file, "a")
    if file then
        file:write(message .. "\n")
        file:close()
    end
end

-- Write to console
function logger.write_to_console(level, message)
    if not logger.config.console_logging then
        return
    end

    local color = logger.colors[logger.get_level_name(level)] or ""
    local reset = logger.colors.RESET

    if level >= logger.levels.ERROR then
        io.stderr:write(color .. message .. reset .. "\n")
    else
        io.stdout:write(color .. message .. reset .. "\n")
    end
end

-- Core logging function
function logger.log(level, message, source_info)
    if not logger.config.enabled or level < logger.current_level then
        return
    end

    local formatted = logger.format_message(level, message, source_info)

    logger.write_to_file(formatted)
    logger.write_to_console(level, formatted)
end

-- Get source information
function logger.get_source_info(stack_level)
    stack_level = stack_level or 3
    local info = debug.getinfo(stack_level, "Sl")
    if info then
        return {
            file = path.filename(info.source:sub(2)), -- Remove @ prefix
            line = info.currentline
        }
    end
    return nil
end

-- Logging level functions
function logger.trace(message)
    logger.log(logger.levels.TRACE, message, logger.get_source_info())
end

function logger.debug(message)
    logger.log(logger.levels.DEBUG, message, logger.get_source_info())
end

function logger.info(message)
    logger.log(logger.levels.INFO, message, logger.get_source_info())
end

function logger.warn(message)
    logger.log(logger.levels.WARN, message, logger.get_source_info())
end

function logger.error(message)
    logger.log(logger.levels.ERROR, message, logger.get_source_info())
end

function logger.fatal(message)
    logger.log(logger.levels.FATAL, message, logger.get_source_info())
end

-- Error handling utilities
function logger.handle_error(error_msg, context)
    context = context or {}

    local full_message = "Error: " .. error_msg
    if context.module then
        full_message = full_message .. " (Module: " .. context.module .. ")"
    end
    if context.function_name then
        full_message = full_message .. " (Function: " .. context.function_name .. ")"
    end
    if context.details then
        full_message = full_message .. " (Details: " .. context.details .. ")"
    end

    logger.error(full_message)

    -- Return error object
    return {
        message = error_msg,
        context = context,
        timestamp = os.time(),
        handled = true
    }
end

-- Try-catch wrapper
function logger.try_catch(try_func, catch_func, finally_func)
    local success, result = pcall(try_func)

    if not success then
        logger.error("Exception caught: " .. tostring(result))
        if catch_func then
            result = catch_func(result)
        end
    end

    if finally_func then
        finally_func()
    end

    return success, result
end

-- Validate function parameters
function logger.validate_params(params, schema, function_name)
    function_name = function_name or "unknown"

    for param_name, param_schema in pairs(schema) do
        local value = params[param_name]

        -- Check required parameters
        if param_schema.required and value == nil then
            local error_msg = "Missing required parameter '" .. param_name .. "' in " .. function_name
            logger.handle_error(error_msg, {function_name = function_name})
            return false, error_msg
        end

        -- Check parameter types
        if value ~= nil and param_schema.type then
            local actual_type = type(value)
            if actual_type ~= param_schema.type then
                local error_msg = "Invalid type for parameter '" .. param_name .. "' in " .. function_name ..
                                 " (expected " .. param_schema.type .. ", got " .. actual_type .. ")"
                logger.handle_error(error_msg, {function_name = function_name})
                return false, error_msg
            end
        end

        -- Check parameter values
        if value ~= nil and param_schema.values then
            local valid = false
            for _, valid_value in ipairs(param_schema.values) do
                if value == valid_value then
                    valid = true
                    break
                end
            end
            if not valid then
                local error_msg = "Invalid value for parameter '" .. param_name .. "' in " .. function_name ..
                                 " (got " .. tostring(value) .. ")"
                logger.handle_error(error_msg, {function_name = function_name})
                return false, error_msg
            end
        end
    end

    return true
end

-- Log file rotation
function logger.rotate_logs()
    if not os.isfile(logger.config.log_file) then
        return
    end

    local stat = os.stat(logger.config.log_file)
    if stat and stat.size > logger.config.max_file_size then
        -- Rotate existing backup files
        for i = logger.config.backup_count - 1, 1, -1 do
            local old_file = logger.config.log_file .. "." .. i
            local new_file = logger.config.log_file .. "." .. (i + 1)
            if os.isfile(old_file) then
                os.mv(old_file, new_file)
            end
        end

        -- Move current log to backup
        os.mv(logger.config.log_file, logger.config.log_file .. ".1")
    end
end

-- Performance monitoring
logger.performance = {}

function logger.start_timer(name)
    logger.performance[name] = os.clock()
    logger.debug("Timer started: " .. name)
end

function logger.end_timer(name)
    local start_time = logger.performance[name]
    if start_time then
        local elapsed = os.clock() - start_time
        logger.info("Timer '" .. name .. "' elapsed: " .. string.format("%.3f", elapsed) .. "s")
        logger.performance[name] = nil
        return elapsed
    else
        logger.warn("Timer '" .. name .. "' was not started")
        return nil
    end
end

-- Memory usage monitoring
function logger.log_memory_usage(context)
    local meminfo = os.meminfo()
    if meminfo then
        local used_mb = math.floor((meminfo.totalram - meminfo.freeram) / (1024 * 1024))
        local total_mb = math.floor(meminfo.totalram / (1024 * 1024))
        local usage_percent = math.floor((used_mb / total_mb) * 100)

        logger.info("Memory usage" .. (context and " (" .. context .. ")" or "") ..
                   ": " .. used_mb .. "MB / " .. total_mb .. "MB (" .. usage_percent .. "%)")
    end
end

-- Export the module
return logger
