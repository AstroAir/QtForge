# Serialization

QtForge provides a comprehensive serialization framework for converting objects to and from various formats including JSON, XML, binary, and custom formats.

## Overview

The serialization system enables:
- **Multi-format Support**: JSON, XML, binary, and custom serialization formats
- **Type Safety**: Compile-time type checking and automatic type deduction
- **Performance**: Efficient serialization with minimal overhead
- **Extensibility**: Easy addition of custom types and formats
- **Versioning**: Support for schema evolution and backward compatibility
- **Streaming**: Support for large data sets through streaming serialization

## Core Interfaces

### ISerializer Interface

```cpp
namespace qtforge::serialization {

template<typename T>
class ISerializer {
public:
    virtual ~ISerializer() = default;
    
    // Serialization
    virtual expected<std::string, Error> serialize(const T& object) = 0;
    virtual expected<std::vector<uint8_t>, Error> serializeBinary(const T& object) = 0;
    virtual expected<void, Error> serializeToStream(const T& object, std::ostream& stream) = 0;
    
    // Deserialization
    virtual expected<T, Error> deserialize(const std::string& data) = 0;
    virtual expected<T, Error> deserializeBinary(const std::vector<uint8_t>& data) = 0;
    virtual expected<T, Error> deserializeFromStream(std::istream& stream) = 0;
    
    // Metadata
    virtual std::string getFormat() const = 0;
    virtual std::string getVersion() const = 0;
    virtual bool supportsStreaming() const = 0;
};

} // namespace qtforge::serialization
```

### Serializable Interface

```cpp
namespace qtforge::serialization {

class ISerializable {
public:
    virtual ~ISerializable() = default;
    
    // Serialization methods
    virtual expected<nlohmann::json, Error> toJson() const = 0;
    virtual expected<void, Error> fromJson(const nlohmann::json& json) = 0;
    
    // Binary serialization
    virtual expected<std::vector<uint8_t>, Error> toBinary() const = 0;
    virtual expected<void, Error> fromBinary(const std::vector<uint8_t>& data) = 0;
    
    // Metadata
    virtual std::string getTypeName() const = 0;
    virtual uint32_t getVersion() const = 0;
};

} // namespace qtforge::serialization
```

## JSON Serialization

### JsonSerializer

```cpp
template<typename T>
class JsonSerializer : public ISerializer<T> {
public:
    JsonSerializer() = default;
    explicit JsonSerializer(const JsonSerializationOptions& options);
    
    // ISerializer implementation
    expected<std::string, Error> serialize(const T& object) override {
        try {
            nlohmann::json json;
            
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                auto result = object.toJson();
                if (!result) {
                    return result.error();
                }
                json = result.value();
            } else {
                json = object; // Use nlohmann::json automatic conversion
            }
            
            // Add metadata
            if (options_.includeMetadata) {
                json["__metadata"] = createMetadata(object);
            }
            
            return json.dump(options_.indentation);
            
        } catch (const std::exception& e) {
            return Error("JSON serialization failed: " + std::string(e.what()));
        }
    }
    
    expected<T, Error> deserialize(const std::string& data) override {
        try {
            auto json = nlohmann::json::parse(data);
            
            // Validate metadata if present
            if (json.contains("__metadata")) {
                auto validation = validateMetadata(json["__metadata"]);
                if (!validation) {
                    return validation.error();
                }
                json.erase("__metadata");
            }
            
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                T object;
                auto result = object.fromJson(json);
                if (!result) {
                    return result.error();
                }
                return object;
            } else {
                return json.get<T>();
            }
            
        } catch (const std::exception& e) {
            return Error("JSON deserialization failed: " + std::string(e.what()));
        }
    }
    
    std::string getFormat() const override { return "JSON"; }
    std::string getVersion() const override { return "1.0"; }
    bool supportsStreaming() const override { return true; }

private:
    struct JsonSerializationOptions {
        int indentation = 2;
        bool includeMetadata = true;
        bool validateSchema = false;
        bool allowComments = false;
        std::string schemaVersion = "1.0";
    };
    
    JsonSerializationOptions options_;
    
    nlohmann::json createMetadata(const T& object) {
        nlohmann::json metadata;
        metadata["type"] = typeid(T).name();
        metadata["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        metadata["serializer"] = "JsonSerializer";
        metadata["version"] = getVersion();
        
        if constexpr (std::is_base_of_v<ISerializable, T>) {
            metadata["object_type"] = object.getTypeName();
            metadata["object_version"] = object.getVersion();
        }
        
        return metadata;
    }
    
    expected<void, Error> validateMetadata(const nlohmann::json& metadata) {
        if (!metadata.contains("type") || !metadata.contains("version")) {
            return Error("Invalid metadata: missing required fields");
        }
        
        std::string type = metadata["type"];
        if (type != typeid(T).name()) {
            return Error("Type mismatch: expected " + std::string(typeid(T).name()) + 
                        ", got " + type);
        }
        
        return {};
    }
};
```

### JSON Serialization Examples

```cpp
// Example serializable class
class UserProfile : public qtforge::serialization::ISerializable {
public:
    std::string username;
    std::string email;
    int age;
    std::vector<std::string> interests;
    std::map<std::string, std::string> preferences;
    
    expected<nlohmann::json, Error> toJson() const override {
        nlohmann::json json;
        json["username"] = username;
        json["email"] = email;
        json["age"] = age;
        json["interests"] = interests;
        json["preferences"] = preferences;
        return json;
    }
    
    expected<void, Error> fromJson(const nlohmann::json& json) override {
        try {
            username = json.at("username").get<std::string>();
            email = json.at("email").get<std::string>();
            age = json.at("age").get<int>();
            interests = json.at("interests").get<std::vector<std::string>>();
            preferences = json.at("preferences").get<std::map<std::string, std::string>>();
            return {};
        } catch (const std::exception& e) {
            return Error("Failed to deserialize UserProfile: " + std::string(e.what()));
        }
    }
    
    std::string getTypeName() const override { return "UserProfile"; }
    uint32_t getVersion() const override { return 1; }
};

// Usage example
void jsonSerializationExample() {
    UserProfile profile;
    profile.username = "john_doe";
    profile.email = "john@example.com";
    profile.age = 30;
    profile.interests = {"programming", "music", "travel"};
    profile.preferences = {{"theme", "dark"}, {"language", "en"}};
    
    // Serialize
    JsonSerializer<UserProfile> serializer;
    auto serialized = serializer.serialize(profile);
    
    if (serialized) {
        std::cout << "Serialized JSON:\n" << serialized.value() << std::endl;
        
        // Deserialize
        auto deserialized = serializer.deserialize(serialized.value());
        if (deserialized) {
            std::cout << "Deserialized username: " << deserialized.value().username << std::endl;
        }
    }
}
```

## Binary Serialization

### BinarySerializer

```cpp
template<typename T>
class BinarySerializer : public ISerializer<T> {
public:
    BinarySerializer() = default;
    explicit BinarySerializer(const BinarySerializationOptions& options);
    
    expected<std::vector<uint8_t>, Error> serializeBinary(const T& object) override {
        try {
            BinaryWriter writer;
            
            // Write header
            writer.writeHeader(createHeader(object));
            
            // Write object data
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                auto binaryData = object.toBinary();
                if (!binaryData) {
                    return binaryData.error();
                }
                writer.writeBytes(binaryData.value());
            } else {
                writer.writeObject(object);
            }
            
            // Write footer (checksum, etc.)
            if (options_.includeChecksum) {
                writer.writeChecksum();
            }
            
            return writer.getData();
            
        } catch (const std::exception& e) {
            return Error("Binary serialization failed: " + std::string(e.what()));
        }
    }
    
    expected<T, Error> deserializeBinary(const std::vector<uint8_t>& data) override {
        try {
            BinaryReader reader(data);
            
            // Read and validate header
            auto header = reader.readHeader();
            auto validation = validateHeader(header);
            if (!validation) {
                return validation.error();
            }
            
            // Read object data
            T object;
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                auto objectData = reader.readBytes(header.dataSize);
                auto result = object.fromBinary(objectData);
                if (!result) {
                    return result.error();
                }
            } else {
                object = reader.readObject<T>();
            }
            
            // Validate checksum if present
            if (options_.includeChecksum) {
                auto checksumValidation = reader.validateChecksum();
                if (!checksumValidation) {
                    return checksumValidation.error();
                }
            }
            
            return object;
            
        } catch (const std::exception& e) {
            return Error("Binary deserialization failed: " + std::string(e.what()));
        }
    }
    
    std::string getFormat() const override { return "Binary"; }
    std::string getVersion() const override { return "1.0"; }
    bool supportsStreaming() const override { return true; }

private:
    struct BinarySerializationOptions {
        bool includeChecksum = true;
        bool compress = false;
        bool encrypt = false;
        std::string encryptionKey;
        CompressionAlgorithm compression = CompressionAlgorithm::None;
    };
    
    struct BinaryHeader {
        uint32_t magic = 0x51544647; // "QTFG"
        uint32_t version = 1;
        uint32_t dataSize = 0;
        uint32_t flags = 0;
        char typeName[64] = {0};
        uint64_t timestamp = 0;
    };
    
    BinarySerializationOptions options_;
    
    BinaryHeader createHeader(const T& object) {
        BinaryHeader header;
        header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        std::string typeName = typeid(T).name();
        std::strncpy(header.typeName, typeName.c_str(), sizeof(header.typeName) - 1);
        
        if (options_.compress) header.flags |= 0x01;
        if (options_.encrypt) header.flags |= 0x02;
        if (options_.includeChecksum) header.flags |= 0x04;
        
        return header;
    }
    
    expected<void, Error> validateHeader(const BinaryHeader& header) {
        if (header.magic != 0x51544647) {
            return Error("Invalid binary format: magic number mismatch");
        }
        
        if (header.version > 1) {
            return Error("Unsupported binary format version: " + std::to_string(header.version));
        }
        
        std::string expectedType = typeid(T).name();
        if (std::string(header.typeName) != expectedType) {
            return Error("Type mismatch: expected " + expectedType + 
                        ", got " + std::string(header.typeName));
        }
        
        return {};
    }
};
```

## XML Serialization

### XmlSerializer

```cpp
template<typename T>
class XmlSerializer : public ISerializer<T> {
public:
    XmlSerializer() = default;
    explicit XmlSerializer(const XmlSerializationOptions& options);
    
    expected<std::string, Error> serialize(const T& object) override {
        try {
            pugi::xml_document doc;
            
            // Create root element
            auto root = doc.append_child("object");
            root.append_attribute("type").set_value(typeid(T).name());
            root.append_attribute("version").set_value(getVersion().c_str());
            
            // Serialize object
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                auto jsonResult = object.toJson();
                if (!jsonResult) {
                    return jsonResult.error();
                }
                
                auto xmlResult = jsonToXml(jsonResult.value(), root);
                if (!xmlResult) {
                    return xmlResult.error();
                }
            } else {
                auto xmlResult = objectToXml(object, root);
                if (!xmlResult) {
                    return xmlResult.error();
                }
            }
            
            // Convert to string
            std::ostringstream oss;
            doc.save(oss, "  ");
            return oss.str();
            
        } catch (const std::exception& e) {
            return Error("XML serialization failed: " + std::string(e.what()));
        }
    }
    
    expected<T, Error> deserialize(const std::string& data) override {
        try {
            pugi::xml_document doc;
            auto result = doc.load_string(data.c_str());
            
            if (!result) {
                return Error("XML parsing failed: " + std::string(result.description()));
            }
            
            auto root = doc.child("object");
            if (!root) {
                return Error("Invalid XML format: missing root element");
            }
            
            // Validate type
            std::string type = root.attribute("type").value();
            if (type != typeid(T).name()) {
                return Error("Type mismatch: expected " + std::string(typeid(T).name()) + 
                            ", got " + type);
            }
            
            // Deserialize object
            T object;
            if constexpr (std::is_base_of_v<ISerializable, T>) {
                auto json = xmlToJson(root);
                if (!json) {
                    return json.error();
                }
                
                auto fromJsonResult = object.fromJson(json.value());
                if (!fromJsonResult) {
                    return fromJsonResult.error();
                }
            } else {
                auto fromXmlResult = xmlToObject<T>(root);
                if (!fromXmlResult) {
                    return fromXmlResult.error();
                }
                object = fromXmlResult.value();
            }
            
            return object;
            
        } catch (const std::exception& e) {
            return Error("XML deserialization failed: " + std::string(e.what()));
        }
    }
    
    std::string getFormat() const override { return "XML"; }
    std::string getVersion() const override { return "1.0"; }
    bool supportsStreaming() const override { return false; }

private:
    struct XmlSerializationOptions {
        std::string encoding = "UTF-8";
        bool prettyPrint = true;
        std::string indentation = "  ";
        bool includeDeclaration = true;
        bool validateSchema = false;
        std::string schemaPath;
    };
    
    XmlSerializationOptions options_;
    
    expected<void, Error> jsonToXml(const nlohmann::json& json, pugi::xml_node& parent) {
        // Convert JSON to XML representation
        // Implementation details...
        return {};
    }
    
    expected<nlohmann::json, Error> xmlToJson(const pugi::xml_node& node) {
        // Convert XML to JSON representation
        // Implementation details...
        return nlohmann::json{};
    }
};
```

## Serialization Factory

### SerializerFactory

```cpp
class SerializerFactory {
public:
    enum class Format {
        JSON,
        XML,
        Binary,
        MessagePack,
        CBOR,
        Custom
    };
    
    template<typename T>
    static std::unique_ptr<ISerializer<T>> createSerializer(Format format) {
        switch (format) {
            case Format::JSON:
                return std::make_unique<JsonSerializer<T>>();
            case Format::XML:
                return std::make_unique<XmlSerializer<T>>();
            case Format::Binary:
                return std::make_unique<BinarySerializer<T>>();
            case Format::MessagePack:
                return std::make_unique<MessagePackSerializer<T>>();
            case Format::CBOR:
                return std::make_unique<CborSerializer<T>>();
            default:
                return nullptr;
        }
    }
    
    template<typename T>
    static void registerCustomSerializer(const std::string& name, 
                                       std::function<std::unique_ptr<ISerializer<T>>()> factory) {
        customFactories_[typeid(T).name()][name] = [factory]() -> std::unique_ptr<void> {
            return std::unique_ptr<void>(factory().release());
        };
    }
    
    template<typename T>
    static std::unique_ptr<ISerializer<T>> createCustomSerializer(const std::string& name) {
        auto typeIt = customFactories_.find(typeid(T).name());
        if (typeIt != customFactories_.end()) {
            auto factoryIt = typeIt->second.find(name);
            if (factoryIt != typeIt->second.end()) {
                auto ptr = factoryIt->second();
                return std::unique_ptr<ISerializer<T>>(static_cast<ISerializer<T>*>(ptr.release()));
            }
        }
        return nullptr;
    }

private:
    static std::unordered_map<std::string, 
                            std::unordered_map<std::string, 
                                             std::function<std::unique_ptr<void>()>>> customFactories_;
};
```

## Advanced Features

### Streaming Serialization

```cpp
template<typename T>
class StreamingJsonSerializer {
public:
    expected<void, Error> serializeArray(const std::vector<T>& objects, std::ostream& stream) {
        try {
            stream << "[\n";
            
            for (size_t i = 0; i < objects.size(); ++i) {
                if (i > 0) {
                    stream << ",\n";
                }
                
                auto serialized = JsonSerializer<T>().serialize(objects[i]);
                if (!serialized) {
                    return serialized.error();
                }
                
                stream << "  " << serialized.value();
            }
            
            stream << "\n]";
            return {};
            
        } catch (const std::exception& e) {
            return Error("Streaming serialization failed: " + std::string(e.what()));
        }
    }
    
    expected<std::vector<T>, Error> deserializeArray(std::istream& stream) {
        try {
            std::string content((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
            
            auto json = nlohmann::json::parse(content);
            if (!json.is_array()) {
                return Error("Expected JSON array");
            }
            
            std::vector<T> objects;
            objects.reserve(json.size());
            
            for (const auto& item : json) {
                auto serialized = item.dump();
                auto deserialized = JsonSerializer<T>().deserialize(serialized);
                if (!deserialized) {
                    return deserialized.error();
                }
                objects.push_back(deserialized.value());
            }
            
            return objects;
            
        } catch (const std::exception& e) {
            return Error("Streaming deserialization failed: " + std::string(e.what()));
        }
    }
};
```

### Schema Validation

```cpp
class SchemaValidator {
public:
    expected<void, Error> validateJson(const nlohmann::json& data, const std::string& schemaPath) {
        try {
            // Load schema
            std::ifstream schemaFile(schemaPath);
            if (!schemaFile.is_open()) {
                return Error("Failed to open schema file: " + schemaPath);
            }
            
            nlohmann::json schema;
            schemaFile >> schema;
            
            // Validate against schema
            nlohmann::json_schema::json_validator validator;
            validator.set_root_schema(schema);
            
            auto validation = validator.validate(data);
            if (!validation.empty()) {
                std::ostringstream oss;
                oss << "Schema validation failed:\n";
                for (const auto& error : validation) {
                    oss << "  " << error.message << "\n";
                }
                return Error(oss.str());
            }
            
            return {};
            
        } catch (const std::exception& e) {
            return Error("Schema validation error: " + std::string(e.what()));
        }
    }
};
```

## Usage Examples

### Complete Serialization Example

```cpp
void comprehensiveSerializationExample() {
    // Create test data
    UserProfile profile;
    profile.username = "alice";
    profile.email = "alice@example.com";
    profile.age = 28;
    profile.interests = {"reading", "hiking", "photography"};
    profile.preferences = {{"theme", "light"}, {"notifications", "enabled"}};
    
    // Test different serialization formats
    std::cout << "=== JSON Serialization ===" << std::endl;
    auto jsonSerializer = SerializerFactory::createSerializer<UserProfile>(SerializerFactory::Format::JSON);
    auto jsonResult = jsonSerializer->serialize(profile);
    if (jsonResult) {
        std::cout << jsonResult.value() << std::endl;
        
        auto deserialized = jsonSerializer->deserialize(jsonResult.value());
        if (deserialized) {
            std::cout << "Deserialized username: " << deserialized.value().username << std::endl;
        }
    }
    
    std::cout << "\n=== Binary Serialization ===" << std::endl;
    auto binarySerializer = SerializerFactory::createSerializer<UserProfile>(SerializerFactory::Format::Binary);
    auto binaryResult = binarySerializer->serializeBinary(profile);
    if (binaryResult) {
        std::cout << "Binary size: " << binaryResult.value().size() << " bytes" << std::endl;
        
        auto deserialized = binarySerializer->deserializeBinary(binaryResult.value());
        if (deserialized) {
            std::cout << "Deserialized username: " << deserialized.value().username << std::endl;
        }
    }
    
    std::cout << "\n=== XML Serialization ===" << std::endl;
    auto xmlSerializer = SerializerFactory::createSerializer<UserProfile>(SerializerFactory::Format::XML);
    auto xmlResult = xmlSerializer->serialize(profile);
    if (xmlResult) {
        std::cout << xmlResult.value() << std::endl;
    }
}
```

## Best Practices

### 1. Performance Optimization

- **Choose Appropriate Format**: Use binary for performance, JSON for readability
- **Streaming**: Use streaming for large datasets
- **Compression**: Enable compression for network transmission
- **Memory Management**: Minimize memory allocations during serialization

### 2. Versioning and Compatibility

- **Schema Evolution**: Design schemas to support backward compatibility
- **Version Fields**: Include version information in serialized data
- **Migration**: Provide migration paths for schema changes
- **Testing**: Test serialization with different versions

### 3. Error Handling

- **Validation**: Validate data before and after serialization
- **Error Recovery**: Provide meaningful error messages
- **Partial Failures**: Handle partial serialization failures gracefully
- **Logging**: Log serialization errors for debugging

## See Also

- **[Message Types](../communication/message-types.md)**: Message serialization patterns
- **[Error Handling](error-handling.md)**: Error handling in serialization
- **[Performance Optimization](../../user-guide/performance-optimization.md)**: Serialization performance
- **[Best Practices](../../developer-guide/best-practices.md)**: Serialization best practices
