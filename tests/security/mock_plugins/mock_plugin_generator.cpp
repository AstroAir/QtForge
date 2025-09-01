/**
 * @file mock_plugin_generator.cpp
 * @brief Generator for mock plugins used in sandbox testing
 * @version 3.2.0
 */

#include "mock_plugin_generator.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

MockPluginGenerator::MockPluginGenerator(const QString& output_dir,
                                         QObject* parent)
    : QObject(parent), m_output_dir(output_dir) {
    // Ensure output directory exists
    QDir().mkpath(m_output_dir);
}

MockPluginGenerator::~MockPluginGenerator() { cleanup(); }

QString MockPluginGenerator::createBehavingPlugin(const QString& name) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import time
import json

def main():
    print(f"Mock plugin '{name}' started")

    # Simulate some work
    for i in range(5):
        print(f"Processing step {i+1}/5")
        time.sleep(0.1)

    # Output result
    result = {
        "status": "success",
        "plugin_name": "{name}",
        "steps_completed": 5,
        "message": "Plugin executed successfully"
    }

    print(json.dumps(result))
    print(f"Mock plugin '{name}' completed successfully")
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);
    return createPythonPlugin(name + "_behaving", script_content);
}

QString MockPluginGenerator::createResourceHungryPlugin(const QString& name) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import time
import json

def main():
    print(f"Resource-hungry plugin '{name}' started")

    # Consume memory
    data = []
    try:
        for i in range(100000):
            data.append("x" * 1000)  # Allocate memory
            if i % 10000 == 0:
                print(f"Allocated {i * 1000} bytes")
                time.sleep(0.01)  # Also consume CPU time
    except MemoryError:
        print("Memory limit reached")

    # Try to consume more CPU
    start_time = time.time()
    while time.time() - start_time < 2.0:
        # Busy loop
        sum(range(1000))

    result = {
        "status": "completed",
        "plugin_name": "{name}",
        "memory_allocated": len(data),
        "message": "Resource consumption completed"
    }

    print(json.dumps(result))
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);
    return createPythonPlugin(name + "_resource_hungry", script_content);
}

QString MockPluginGenerator::createMaliciousPlugin(const QString& name) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import os
import time
import json

def main():
    print(f"Malicious plugin '{name}' started")

    violations = []

    # Attempt unauthorized file access
    try:
        with open('/etc/passwd', 'r') as f:
            content = f.read()
        violations.append("unauthorized_file_read")
        print("WARNING: Unauthorized file access succeeded!")
    except Exception as e:
        print(f"File access blocked: {e}")

    # Attempt to execute system commands
    try:
        result = os.system('whoami')
        if result == 0:
            violations.append("system_command_execution")
            print("WARNING: System command execution succeeded!")
    except Exception as e:
        print(f"System command blocked: {e}")

    # Attempt to create processes
    try:
        import subprocess
        result = subprocess.run(['ls', '-la'], capture_output=True, text=True)
        violations.append("process_creation")
        print("WARNING: Process creation succeeded!")
    except Exception as e:
        print(f"Process creation blocked: {e}")

    # Attempt network access
    try:
        import urllib.request
        response = urllib.request.urlopen('http://example.com', timeout=1)
        violations.append("network_access")
        print("WARNING: Network access succeeded!")
    except Exception as e:
        print(f"Network access blocked: {e}")

    result = {
        "status": "completed",
        "plugin_name": "{name}",
        "violations_attempted": ["file_access", "system_commands", "process_creation", "network_access"],
        "violations_succeeded": violations,
        "message": "Security test completed"
    }

    print(json.dumps(result))
    return len(violations)  # Return number of successful violations

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);
    return createPythonPlugin(name + "_malicious", script_content);
}

QString MockPluginGenerator::createCrashingPlugin(const QString& name) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import time
import random

def main():
    print(f"Crashing plugin '{name}' started")

    # Do some work before crashing
    for i in range(3):
        print(f"Working... step {i+1}")
        time.sleep(0.1)

    # Randomly choose crash type
    crash_type = random.randint(1, 4)

    if crash_type == 1:
        print("Triggering division by zero")
        result = 1 / 0
    elif crash_type == 2:
        print("Triggering null pointer access")
        none_obj = None
        none_obj.some_method()
    elif crash_type == 3:
        print("Triggering index out of bounds")
        empty_list = []
        value = empty_list[10]
    else:
        print("Triggering assertion error")
        assert False, "Intentional crash"

    # This should never be reached
    print("ERROR: Plugin should have crashed!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);
    return createPythonPlugin(name + "_crashing", script_content);
}

QString MockPluginGenerator::createLongRunningPlugin(const QString& name,
                                                     int duration_seconds) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import time
import json

def main():
    print(f"Long-running plugin '{name}' started")
    duration = {duration}

    start_time = time.time()
    step_duration = duration / 10

    for i in range(10):
        print(f"Long operation step {i+1}/10")
        time.sleep(step_duration)

        elapsed = time.time() - start_time
        print(f"Elapsed time: {elapsed:.1f}s / {duration}s")

    result = {
        "status": "completed",
        "plugin_name": "{name}",
        "duration": duration,
        "message": f"Long operation completed in {duration} seconds"
    }

    print(json.dumps(result))
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);
    script_content.replace("{duration}", QString::number(duration_seconds));
    return createPythonPlugin(name + "_long_running", script_content);
}

QString MockPluginGenerator::createFileAccessPlugin(
    const QString& name, const QStringList& file_paths) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import os
import json

def main():
    print(f"File access plugin '{name}' started")

    file_paths = {file_paths}
    results = []

    for file_path in file_paths:
        try:
            if os.path.exists(file_path):
                with open(file_path, 'r') as f:
                    content = f.read()[:100]  # Read first 100 chars
                results.append({
                    "path": file_path,
                    "status": "success",
                    "size": len(content),
                    "preview": content[:50]
                })
                print(f"Successfully read: {file_path}")
            else:
                results.append({
                    "path": file_path,
                    "status": "not_found"
                })
                print(f"File not found: {file_path}")
        except Exception as e:
            results.append({
                "path": file_path,
                "status": "error",
                "error": str(e)
            })
            print(f"Error reading {file_path}: {e}")

    result = {
        "status": "completed",
        "plugin_name": "{name}",
        "files_accessed": results,
        "message": "File access test completed"
    }

    print(json.dumps(result))
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);

    // Convert QStringList to Python list format
    QStringList quoted_paths;
    for (const QString& path : file_paths) {
        quoted_paths.append("\"" + path + "\"");
    }
    script_content.replace("{file_paths}", "[" + quoted_paths.join(", ") + "]");

    return createPythonPlugin(name + "_file_access", script_content);
}

QString MockPluginGenerator::createNetworkAccessPlugin(
    const QString& name, const QStringList& hosts) {
    QString script_content = R"(#!/usr/bin/env python3
import sys
import json

def main():
    print(f"Network access plugin '{name}' started")

    hosts = {hosts}
    results = []

    try:
        import urllib.request
        import socket

        for host in hosts:
            try:
                # Try to connect
                url = f"http://{host}"
                request = urllib.request.Request(url)
                request.add_header('User-Agent', 'MockPlugin/1.0')

                response = urllib.request.urlopen(request, timeout=2)
                status_code = response.getcode()

                results.append({
                    "host": host,
                    "status": "success",
                    "status_code": status_code
                })
                print(f"Successfully connected to: {host}")

            except Exception as e:
                results.append({
                    "host": host,
                    "status": "error",
                    "error": str(e)
                })
                print(f"Error connecting to {host}: {e}")

    except ImportError:
        results.append({
            "error": "Network modules not available"
        })
        print("Network access modules not available")

    result = {
        "status": "completed",
        "plugin_name": "{name}",
        "network_attempts": results,
        "message": "Network access test completed"
    }

    print(json.dumps(result))
    return 0

if __name__ == "__main__":
    sys.exit(main())
)";

    script_content.replace("{name}", name);

    // Convert QStringList to Python list format
    QStringList quoted_hosts;
    for (const QString& host : hosts) {
        quoted_hosts.append("\"" + host + "\"");
    }
    script_content.replace("{hosts}", "[" + quoted_hosts.join(", ") + "]");

    return createPythonPlugin(name + "_network_access", script_content);
}

QStringList MockPluginGenerator::createTestSuite(const QString& suite_name) {
    QStringList plugins;

    // Create a comprehensive test suite
    plugins.append(createBehavingPlugin(suite_name + "_good"));
    plugins.append(createResourceHungryPlugin(suite_name + "_hungry"));
    plugins.append(createMaliciousPlugin(suite_name + "_malicious"));
    plugins.append(createCrashingPlugin(suite_name + "_crash"));
    plugins.append(createLongRunningPlugin(suite_name + "_long", 3));

    // File access tests
    QStringList test_files = {
        "/etc/passwd", "/tmp/test_file.txt",
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
            "/sandbox_test.txt"};
    plugins.append(createFileAccessPlugin(suite_name + "_files", test_files));

    // Network access tests
    QStringList test_hosts = {"example.com", "google.com", "localhost"};
    plugins.append(
        createNetworkAccessPlugin(suite_name + "_network", test_hosts));

    return plugins;
}

void MockPluginGenerator::cleanup() {
    // Remove all created plugin files
    for (const QString& plugin_path : m_created_plugins) {
        QFile::remove(plugin_path);
    }
    m_created_plugins.clear();
}

QString MockPluginGenerator::createPythonPlugin(const QString& name,
                                                const QString& script_content) {
    QString file_path = m_output_dir + "/" + name + ".py";

    QFile file(file_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << script_content;
        file.close();

        // Make executable on Unix systems
        QFile::setPermissions(
            file_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                           QFile::ReadGroup | QFile::ExeGroup);

        m_created_plugins.append(file_path);
        return file_path;
    }

    qWarning() << "Failed to create plugin file:" << file_path;
    return QString();
}
