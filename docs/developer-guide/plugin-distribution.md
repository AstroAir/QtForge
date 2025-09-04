# Plugin Distribution

This guide covers the distribution, packaging, and deployment of QtForge plugins for various environments and platforms.

## Overview

Plugin distribution involves:
- **Packaging**: Creating distributable plugin packages
- **Versioning**: Managing plugin versions and dependencies
- **Distribution Channels**: Publishing plugins through various channels
- **Installation**: Automated plugin installation and updates
- **Security**: Signing and verification of plugin packages

## Plugin Packaging

### Basic Plugin Package Structure

```
my-plugin-1.0.0/
├── plugin.json              # Plugin metadata
├── lib/
│   ├── my-plugin.dll        # Main plugin library (Windows)
│   ├── my-plugin.so         # Main plugin library (Linux)
│   └── my-plugin.dylib      # Main plugin library (macOS)
├── resources/
│   ├── icons/
│   ├── translations/
│   └── data/
├── docs/
│   ├── README.md
│   ├── CHANGELOG.md
│   └── LICENSE.md
├── dependencies/
│   └── third-party-libs/
└── scripts/
    ├── install.sh
    ├── uninstall.sh
    └── post-install.py
```

### Plugin Metadata (plugin.json)

```json
{
  "id": "com.example.myplugin",
  "name": "My Plugin",
  "version": "1.0.0",
  "description": "A sample plugin for demonstration",
  "author": "Example Developer",
  "email": "developer@example.com",
  "website": "https://example.com/myplugin",
  "license": "MIT",
  "category": "utility",
  "tags": ["data", "processing", "utility"],
  
  "compatibility": {
    "qtforge_version": ">=1.0.0",
    "qt_version": ">=6.0.0",
    "platforms": ["windows", "linux", "macos"]
  },
  
  "dependencies": [
    {
      "id": "com.example.core",
      "version": ">=1.0.0",
      "optional": false
    },
    {
      "id": "com.example.utils",
      "version": "^2.1.0",
      "optional": true
    }
  ],
  
  "files": {
    "main": "lib/my-plugin.dll",
    "resources": "resources/",
    "documentation": "docs/"
  },
  
  "permissions": [
    "filesystem:read:/tmp",
    "network:connect:api.example.com",
    "system:environment"
  ],
  
  "configuration": {
    "default_settings": {
      "enabled": true,
      "log_level": "info",
      "cache_size": 100
    },
    "schema": "config-schema.json"
  }
}
```

### Automated Packaging Script

```python
#!/usr/bin/env python3
"""
Plugin packaging script for QtForge plugins
"""

import os
import json
import shutil
import zipfile
import hashlib
from pathlib import Path

class PluginPackager:
    def __init__(self, source_dir, output_dir):
        self.source_dir = Path(source_dir)
        self.output_dir = Path(output_dir)
        self.metadata = self.load_metadata()
        
    def load_metadata(self):
        """Load plugin metadata from plugin.json"""
        metadata_file = self.source_dir / "plugin.json"
        if not metadata_file.exists():
            raise FileNotFoundError("plugin.json not found")
            
        with open(metadata_file, 'r') as f:
            return json.load(f)
    
    def validate_metadata(self):
        """Validate plugin metadata"""
        required_fields = ['id', 'name', 'version', 'description', 'author']
        for field in required_fields:
            if field not in self.metadata:
                raise ValueError(f"Required field '{field}' missing from metadata")
        
        # Validate version format
        version = self.metadata['version']
        if not self.is_valid_version(version):
            raise ValueError(f"Invalid version format: {version}")
    
    def is_valid_version(self, version):
        """Check if version follows semantic versioning"""
        import re
        pattern = r'^(\d+)\.(\d+)\.(\d+)(?:-([a-zA-Z0-9\-]+))?(?:\+([a-zA-Z0-9\-]+))?$'
        return re.match(pattern, version) is not None
    
    def create_package(self):
        """Create the plugin package"""
        self.validate_metadata()
        
        plugin_id = self.metadata['id']
        version = self.metadata['version']
        package_name = f"{plugin_id}-{version}.qtpkg"
        package_path = self.output_dir / package_name
        
        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Create package
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as package:
            self.add_files_to_package(package)
            self.add_metadata_to_package(package)
            self.add_checksums_to_package(package)
        
        print(f"Package created: {package_path}")
        return package_path
    
    def add_files_to_package(self, package):
        """Add plugin files to package"""
        for root, dirs, files in os.walk(self.source_dir):
            for file in files:
                file_path = Path(root) / file
                relative_path = file_path.relative_to(self.source_dir)
                package.write(file_path, relative_path)
    
    def add_metadata_to_package(self, package):
        """Add enhanced metadata to package"""
        # Add build information
        build_info = {
            "build_time": datetime.now().isoformat(),
            "build_host": platform.node(),
            "build_user": os.getenv('USER', 'unknown'),
            "qtforge_version": self.get_qtforge_version()
        }
        
        enhanced_metadata = {
            **self.metadata,
            "build_info": build_info
        }
        
        metadata_json = json.dumps(enhanced_metadata, indent=2)
        package.writestr("META-INF/plugin.json", metadata_json)
    
    def add_checksums_to_package(self, package):
        """Add file checksums for integrity verification"""
        checksums = {}
        
        for info in package.infolist():
            if info.filename.startswith("META-INF/"):
                continue
                
            with package.open(info) as f:
                content = f.read()
                checksums[info.filename] = hashlib.sha256(content).hexdigest()
        
        checksums_json = json.dumps(checksums, indent=2)
        package.writestr("META-INF/checksums.json", checksums_json)
    
    def get_qtforge_version(self):
        """Get QtForge version"""
        # Implementation to get QtForge version
        return "1.0.0"  # Placeholder

# Usage
if __name__ == "__main__":
    import sys
    import datetime
    import platform
    
    if len(sys.argv) != 3:
        print("Usage: package_plugin.py <source_dir> <output_dir>")
        sys.exit(1)
    
    source_dir = sys.argv[1]
    output_dir = sys.argv[2]
    
    packager = PluginPackager(source_dir, output_dir)
    packager.create_package()
```

## Distribution Channels

### Plugin Registry

```cpp
class PluginRegistry {
public:
    struct PluginEntry {
        QString id;
        QString name;
        QString version;
        QString description;
        QString author;
        QUrl downloadUrl;
        QUrl documentationUrl;
        QStringList tags;
        QDateTime publishDate;
        QDateTime lastUpdate;
        int downloadCount;
        double rating;
        bool verified;
    };
    
    bool publishPlugin(const PluginEntry& entry, const QByteArray& packageData) {
        // Validate plugin package
        if (!validatePackage(packageData)) {
            return false;
        }
        
        // Store plugin in registry
        QString storagePath = getPluginStoragePath(entry.id, entry.version);
        if (!storePluginPackage(storagePath, packageData)) {
            return false;
        }
        
        // Update registry database
        updateRegistryDatabase(entry);
        
        // Generate plugin index
        generatePluginIndex();
        
        emit pluginPublished(entry.id, entry.version);
        return true;
    }
    
    QList<PluginEntry> searchPlugins(const QString& query, 
                                   const QStringList& tags = {},
                                   const QString& category = {}) {
        QList<PluginEntry> results;
        
        // Search in database
        QSqlQuery sqlQuery(m_database);
        QString sql = "SELECT * FROM plugins WHERE 1=1";
        
        if (!query.isEmpty()) {
            sql += " AND (name LIKE ? OR description LIKE ? OR tags LIKE ?)";
        }
        
        if (!category.isEmpty()) {
            sql += " AND category = ?";
        }
        
        sqlQuery.prepare(sql);
        
        if (!query.isEmpty()) {
            QString searchPattern = QString("%%1%").arg(query);
            sqlQuery.addBindValue(searchPattern);
            sqlQuery.addBindValue(searchPattern);
            sqlQuery.addBindValue(searchPattern);
        }
        
        if (!category.isEmpty()) {
            sqlQuery.addBindValue(category);
        }
        
        if (sqlQuery.exec()) {
            while (sqlQuery.next()) {
                results.append(createPluginEntryFromQuery(sqlQuery));
            }
        }
        
        return results;
    }
    
signals:
    void pluginPublished(const QString& pluginId, const QString& version);
    void pluginUpdated(const QString& pluginId, const QString& version);
    void pluginRemoved(const QString& pluginId);

private:
    bool validatePackage(const QByteArray& packageData);
    QString getPluginStoragePath(const QString& id, const QString& version);
    bool storePluginPackage(const QString& path, const QByteArray& data);
    void updateRegistryDatabase(const PluginEntry& entry);
    void generatePluginIndex();
    PluginEntry createPluginEntryFromQuery(const QSqlQuery& query);
    
    QSqlDatabase m_database;
};
```

### Package Repository

```cpp
class PackageRepository {
public:
    enum RepositoryType {
        Official,
        Community,
        Enterprise,
        Private
    };
    
    struct Repository {
        QString name;
        QUrl baseUrl;
        RepositoryType type;
        QString description;
        bool enabled;
        QString authToken;
        QDateTime lastSync;
    };
    
    void addRepository(const Repository& repo) {
        m_repositories[repo.name] = repo;
        syncRepository(repo.name);
    }
    
    QList<PluginInfo> getAvailablePlugins(const QString& repositoryName = {}) {
        QList<PluginInfo> plugins;
        
        if (repositoryName.isEmpty()) {
            // Get from all repositories
            for (const auto& repo : m_repositories) {
                if (repo.enabled) {
                    plugins.append(getPluginsFromRepository(repo));
                }
            }
        } else {
            // Get from specific repository
            if (m_repositories.contains(repositoryName)) {
                plugins = getPluginsFromRepository(m_repositories[repositoryName]);
            }
        }
        
        return plugins;
    }
    
    bool downloadPlugin(const QString& pluginId, const QString& version,
                       const QString& destinationPath) {
        // Find plugin in repositories
        for (const auto& repo : m_repositories) {
            if (!repo.enabled) continue;
            
            QUrl downloadUrl = buildDownloadUrl(repo, pluginId, version);
            if (downloadPluginFromUrl(downloadUrl, destinationPath, repo.authToken)) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    void syncRepository(const QString& repositoryName) {
        if (!m_repositories.contains(repositoryName)) {
            return;
        }
        
        Repository& repo = m_repositories[repositoryName];
        QUrl indexUrl = QUrl(repo.baseUrl.toString() + "/index.json");
        
        // Download repository index
        QNetworkRequest request(indexUrl);
        if (!repo.authToken.isEmpty()) {
            request.setRawHeader("Authorization", 
                               QString("Bearer %1").arg(repo.authToken).toUtf8());
        }
        
        QNetworkReply* reply = m_networkManager.get(request);
        connect(reply, &QNetworkReply::finished, [this, reply, repositoryName]() {
            handleRepositorySync(reply, repositoryName);
        });
    }
    
    void handleRepositorySync(QNetworkReply* reply, const QString& repositoryName) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            updateRepositoryIndex(repositoryName, data);
            
            Repository& repo = m_repositories[repositoryName];
            repo.lastSync = QDateTime::currentDateTime();
        }
        
        reply->deleteLater();
    }
    
    QList<PluginInfo> getPluginsFromRepository(const Repository& repo);
    QUrl buildDownloadUrl(const Repository& repo, const QString& pluginId, const QString& version);
    bool downloadPluginFromUrl(const QUrl& url, const QString& destination, const QString& authToken);
    void updateRepositoryIndex(const QString& repositoryName, const QByteArray& indexData);
    
    QMap<QString, Repository> m_repositories;
    QNetworkAccessManager m_networkManager;
};
```

## Plugin Installation

### Automated Installer

```cpp
class PluginInstaller {
public:
    enum InstallationMode {
        UserMode,      // Install for current user
        SystemMode,    // Install system-wide
        PortableMode   // Install in application directory
    };
    
    struct InstallationOptions {
        InstallationMode mode = UserMode;
        bool createDesktopShortcut = false;
        bool registerFileAssociations = false;
        bool startAfterInstall = true;
        QString customInstallPath;
        QVariantMap configOverrides;
    };
    
    bool installPlugin(const QString& packagePath, 
                      const InstallationOptions& options = {}) {
        // Validate package
        if (!validatePackage(packagePath)) {
            emit installationFailed("Invalid package format");
            return false;
        }
        
        // Extract package metadata
        PluginMetadata metadata = extractMetadata(packagePath);
        
        // Check dependencies
        if (!checkDependencies(metadata)) {
            emit installationFailed("Dependency requirements not met");
            return false;
        }
        
        // Determine installation path
        QString installPath = determineInstallPath(metadata, options);
        
        // Extract package
        if (!extractPackage(packagePath, installPath)) {
            emit installationFailed("Failed to extract package");
            return false;
        }
        
        // Run installation scripts
        if (!runInstallationScripts(installPath, metadata)) {
            emit installationFailed("Installation scripts failed");
            rollbackInstallation(installPath);
            return false;
        }
        
        // Register plugin
        if (!registerPlugin(metadata, installPath)) {
            emit installationFailed("Failed to register plugin");
            rollbackInstallation(installPath);
            return false;
        }
        
        // Apply configuration
        applyConfiguration(metadata, options.configOverrides);
        
        emit installationCompleted(metadata.id, metadata.version);
        return true;
    }
    
    bool uninstallPlugin(const QString& pluginId) {
        PluginMetadata metadata = getInstalledPluginMetadata(pluginId);
        if (metadata.id.isEmpty()) {
            return false;
        }
        
        QString installPath = getPluginInstallPath(pluginId);
        
        // Run uninstallation scripts
        runUninstallationScripts(installPath, metadata);
        
        // Unregister plugin
        unregisterPlugin(pluginId);
        
        // Remove files
        removePluginFiles(installPath);
        
        emit uninstallationCompleted(pluginId);
        return true;
    }
    
signals:
    void installationStarted(const QString& pluginId);
    void installationProgress(const QString& pluginId, int percentage);
    void installationCompleted(const QString& pluginId, const QString& version);
    void installationFailed(const QString& error);
    void uninstallationCompleted(const QString& pluginId);

private:
    bool validatePackage(const QString& packagePath);
    PluginMetadata extractMetadata(const QString& packagePath);
    bool checkDependencies(const PluginMetadata& metadata);
    QString determineInstallPath(const PluginMetadata& metadata, const InstallationOptions& options);
    bool extractPackage(const QString& packagePath, const QString& installPath);
    bool runInstallationScripts(const QString& installPath, const PluginMetadata& metadata);
    bool registerPlugin(const PluginMetadata& metadata, const QString& installPath);
    void applyConfiguration(const PluginMetadata& metadata, const QVariantMap& overrides);
    void rollbackInstallation(const QString& installPath);
    
    PluginMetadata getInstalledPluginMetadata(const QString& pluginId);
    QString getPluginInstallPath(const QString& pluginId);
    void runUninstallationScripts(const QString& installPath, const PluginMetadata& metadata);
    void unregisterPlugin(const QString& pluginId);
    void removePluginFiles(const QString& installPath);
};
```

## Plugin Signing and Verification

### Code Signing

```bash
#!/bin/bash
# Plugin signing script

PLUGIN_PATH="$1"
CERTIFICATE_PATH="$2"
PRIVATE_KEY_PATH="$3"
OUTPUT_PATH="$4"

if [ $# -ne 4 ]; then
    echo "Usage: sign_plugin.sh <plugin_path> <certificate> <private_key> <output_path>"
    exit 1
fi

# Create signature
openssl dgst -sha256 -sign "$PRIVATE_KEY_PATH" -out signature.bin "$PLUGIN_PATH"

# Encode signature in base64
base64 signature.bin > signature.b64

# Create signed package
cp "$PLUGIN_PATH" "$OUTPUT_PATH"
zip -u "$OUTPUT_PATH" signature.b64 "$CERTIFICATE_PATH"

# Clean up
rm signature.bin signature.b64

echo "Plugin signed successfully: $OUTPUT_PATH"
```

### Signature Verification

```cpp
class PluginVerifier {
public:
    bool verifyPluginSignature(const QString& packagePath) {
        // Extract signature and certificate from package
        QByteArray signature = extractSignature(packagePath);
        QByteArray certificate = extractCertificate(packagePath);
        
        if (signature.isEmpty() || certificate.isEmpty()) {
            return false;
        }
        
        // Verify certificate chain
        if (!verifyCertificateChain(certificate)) {
            return false;
        }
        
        // Verify signature
        QByteArray packageData = getPackageDataWithoutSignature(packagePath);
        return verifySignature(packageData, signature, certificate);
    }
    
private:
    QByteArray extractSignature(const QString& packagePath);
    QByteArray extractCertificate(const QString& packagePath);
    bool verifyCertificateChain(const QByteArray& certificate);
    QByteArray getPackageDataWithoutSignature(const QString& packagePath);
    bool verifySignature(const QByteArray& data, const QByteArray& signature, const QByteArray& certificate);
};
```

## Best Practices

### Distribution Guidelines

1. **Version Management**: Use semantic versioning for plugins
2. **Dependency Management**: Clearly specify dependencies and version constraints
3. **Platform Support**: Test plugins on all target platforms
4. **Documentation**: Provide comprehensive documentation
5. **Security**: Always sign plugins for distribution

### Packaging Checklist

- [ ] Plugin metadata is complete and valid
- [ ] All dependencies are documented
- [ ] Plugin is tested on target platforms
- [ ] Documentation is included
- [ ] Plugin is signed with valid certificate
- [ ] Installation/uninstallation scripts are tested
- [ ] License information is included
- [ ] Version number follows semantic versioning

## See Also

- [Plugin Development](plugin-development.md)
- [Plugin Marketplace](../api/marketplace/plugin-marketplace.md)
- [Security Configuration](../user-guide/security-configuration.md)
- [Plugin Validation](../api/security/plugin-validator.md)
