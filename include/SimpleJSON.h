#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

/**
 * @brief Simple JSON implementation for the marine generator simulator
 * 
 * This is a basic JSON implementation to avoid external dependencies.
 * It supports the basic types needed for the simulator communication.
 */
class SimpleJSON {
public:
    enum class Type {
        Null,
        String,
        Number,
        Boolean,
        Object,
        Array
    };

    SimpleJSON() : type_(Type::Null) {}
    
    // String constructor
    SimpleJSON(const std::string& value) : type_(Type::String), string_value_(value) {}
    
    // Number constructor
    SimpleJSON(double value) : type_(Type::Number), number_value_(value) {}
    SimpleJSON(int value) : type_(Type::Number), number_value_(static_cast<double>(value)) {}
    SimpleJSON(long long value) : type_(Type::Number), number_value_(static_cast<double>(value)) {}
    
    // Boolean constructor
    SimpleJSON(bool value) : type_(Type::Boolean), bool_value_(value) {}
    
    // Copy constructor
    SimpleJSON(const SimpleJSON& other) = default;
    
    // Assignment operator
    SimpleJSON& operator=(const SimpleJSON& other) = default;

    // Type checking
    bool is_null() const { return type_ == Type::Null; }
    bool is_string() const { return type_ == Type::String; }
    bool is_number() const { return type_ == Type::Number; }
    bool is_boolean() const { return type_ == Type::Boolean; }
    bool is_object() const { return type_ == Type::Object; }
    bool is_array() const { return type_ == Type::Array; }

    // Value access
    std::string as_string() const { return string_value_; }
    double as_number() const { return number_value_; }
    bool as_boolean() const { return bool_value_; }

    // Object operations
    void set(const std::string& key, const SimpleJSON& value) {
        if (type_ != Type::Object) {
            type_ = Type::Object;
            object_values_.clear();
        }
        object_values_[key] = value;
    }

    SimpleJSON get(const std::string& key) const {
        if (type_ == Type::Object) {
            auto it = object_values_.find(key);
            if (it != object_values_.end()) {
                return it->second;
            }
        }
        return SimpleJSON();
    }

    // Array operations
    void push_back(const SimpleJSON& value) {
        if (type_ != Type::Array) {
            type_ = Type::Array;
            array_values_.clear();
        }
        array_values_.push_back(value);
    }

    size_t size() const {
        if (type_ == Type::Array) {
            return array_values_.size();
        }
        return 0;
    }

    SimpleJSON at(size_t index) const {
        if (type_ == Type::Array && index < array_values_.size()) {
            return array_values_[index];
        }
        return SimpleJSON();
    }

    // Serialization
    std::string dump() const {
        std::ostringstream oss;
        dump_to_stream(oss);
        return oss.str();
    }

    // Static factory methods
    static SimpleJSON object() {
        SimpleJSON json;
        json.type_ = Type::Object;
        return json;
    }

    static SimpleJSON array() {
        SimpleJSON json;
        json.type_ = Type::Array;
        return json;
    }

    // Parse JSON string (basic implementation)
    static SimpleJSON parse(const std::string& json_str) {
        // This is a very basic parser - for production use a proper JSON library
        // For now, just return an empty object
        return SimpleJSON::object();
    }

    // Array access operator
    SimpleJSON& operator[](const std::string& key) {
        if (type_ != Type::Object) {
            type_ = Type::Object;
            object_values_.clear();
        }
        return object_values_[key];
    }

    const SimpleJSON& operator[](const std::string& key) const {
        if (type_ == Type::Object) {
            auto it = object_values_.find(key);
            if (it != object_values_.end()) {
                return it->second;
            }
        }
        static SimpleJSON null_json;
        return null_json;
    }

    // Value access with default
    std::string value(const std::string& key, const std::string& default_value = "") const {
        auto json = get(key);
        return json.is_string() ? json.as_string() : default_value;
    }

    double value(const std::string& key, double default_value = 0.0) const {
        auto json = get(key);
        return json.is_number() ? json.as_number() : default_value;
    }

    bool value(const std::string& key, bool default_value = false) const {
        auto json = get(key);
        return json.is_boolean() ? json.as_boolean() : default_value;
    }

private:
    void dump_to_stream(std::ostringstream& oss) const {
        switch (type_) {
            case Type::Null:
                oss << "null";
                break;
            case Type::String:
                oss << "\"" << escape_string(string_value_) << "\"";
                break;
            case Type::Number:
                oss << std::fixed << std::setprecision(6) << number_value_;
                break;
            case Type::Boolean:
                oss << (bool_value_ ? "true" : "false");
                break;
            case Type::Object: {
                oss << "{";
                bool first = true;
                for (const auto& [key, value] : object_values_) {
                    if (!first) oss << ",";
                    oss << "\"" << escape_string(key) << "\":";
                    value.dump_to_stream(oss);
                    first = false;
                }
                oss << "}";
                break;
            }
            case Type::Array:
                oss << "[";
                for (size_t i = 0; i < array_values_.size(); ++i) {
                    if (i > 0) oss << ",";
                    array_values_[i].dump_to_stream(oss);
                }
                oss << "]";
                break;
        }
    }

    std::string escape_string(const std::string& str) const {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }

    Type type_;
    std::string string_value_;
    double number_value_;
    bool bool_value_;
    std::map<std::string, SimpleJSON> object_values_;
    std::vector<SimpleJSON> array_values_;
};
