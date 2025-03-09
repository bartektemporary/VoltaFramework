#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <initializer_list>
#include <cmath>

namespace json {

// Forward declarations
class Value;
class Object;
class Array;

// JSON value types
enum class Type {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

// Exception class for JSON operations
class JsonException : public std::runtime_error {
public:
    explicit JsonException(const std::string& message) : std::runtime_error(message) {}
};

// Base class for all JSON values
class Value {
public:
    virtual ~Value() = default;
    virtual Type type() const = 0;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Value> clone() const = 0;
    
    // Type checking methods
    bool isNull() const { return type() == Type::Null; }
    bool isBoolean() const { return type() == Type::Boolean; }
    bool isNumber() const { return type() == Type::Number; }
    bool isString() const { return type() == Type::String; }
    bool isArray() const { return type() == Type::Array; }
    bool isObject() const { return type() == Type::Object; }
    
    // Casting methods (throw if type doesn't match)
    virtual bool asBoolean() const { 
        throw JsonException("Value is not a boolean");
    }
    
    virtual double asNumber() const { 
        throw JsonException("Value is not a number");
    }
    
    virtual const std::string& asString() const { 
        throw JsonException("Value is not a string");
    }
    
    virtual const Array& asArray() const { 
        throw JsonException("Value is not an array");
    }
    
    virtual Array& asArray() { 
        throw JsonException("Value is not an array");
    }
    
    virtual const Object& asObject() const { 
        throw JsonException("Value is not an object");
    }
    
    virtual Object& asObject() { 
        throw JsonException("Value is not an object");
    }
};

// Null type
class Null : public Value {
public:
    Type type() const override { return Type::Null; }
    
    std::string toString() const override {
        return "null";
    }
    
    std::unique_ptr<Value> clone() const override {
        return std::make_unique<Null>();
    }
};

// Boolean type
class Boolean : public Value {
public:
    explicit Boolean(bool value) : value_(value) {}
    
    Type type() const override { return Type::Boolean; }
    
    bool asBoolean() const override {
        return value_;
    }
    
    std::string toString() const override {
        return value_ ? "true" : "false";
    }
    
    std::unique_ptr<Value> clone() const override {
        return std::make_unique<Boolean>(value_);
    }
    
private:
    bool value_;
};

// Number type
class Number : public Value {
public:
    explicit Number(double value) : value_(value) {}
    
    Type type() const override { return Type::Number; }
    
    double asNumber() const override {
        return value_;
    }
    
    std::string toString() const override {
        // Handle special cases
        if (std::isnan(value_)) return "null";
        if (std::isinf(value_)) {
            return value_ > 0 ? "null" : "null"; // JSON doesn't support infinity
        }
        
        std::ostringstream ss{};
        ss.precision(15); // IEEE double precision
        ss << value_;
        
        // Check if the number is an integer and has no decimal part
        if (static_cast<double>(static_cast<long long>(value_)) == value_) {
            std::string s = ss.str();
            size_t pos = s.find('.');
            if (pos != std::string::npos) {
                return s.substr(0, pos);
            }
        }
        
        return ss.str();
    }
    
    std::unique_ptr<Value> clone() const override {
        return std::make_unique<Number>(value_);
    }
    
private:
    double value_;
};

// String type
class String : public Value {
public:
    explicit String(const std::string& value) : value_(value) {}
    explicit String(std::string&& value) : value_(std::move(value)) {}
    
    Type type() const override { return Type::String; }
    
    const std::string& asString() const override {
        return value_;
    }
    
    std::string toString() const override {
        std::string result{"\""};
        for (char c : value_) {
            switch (c) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '/':  result += "\\/"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        result += buf;
                    } else {
                        result += c;
                    }
            }
        }
        result += "\"";
        return result;
    }
    
    std::unique_ptr<Value> clone() const override {
        return std::make_unique<String>(value_);
    }
    
private:
    std::string value_;
};

// Array type
class Array : public Value {
public:
    Array() = default;
    
    Array(std::initializer_list<std::unique_ptr<Value>> init) {
        for (auto& value : init) {
            if (value) {
                values_.push_back(value->clone());
            }
        }
    }
    
    Type type() const override { return Type::Array; }
    
    const Array& asArray() const override {
        return *this;
    }
    
    Array& asArray() override {
        return *this;
    }
    
    std::string toString() const override {
        std::string result{"["};
        bool first = true;
        
        for (const auto& value : values_) {
            if (!first) {
                result += ",";
            }
            first = false;
            result += value->toString();
        }
        
        result += "]";
        return result;
    }
    
    std::unique_ptr<Value> clone() const override {
        auto result = std::make_unique<Array>();
        for (const auto& value : values_) {
            result->values_.push_back(value->clone());
        }
        return result;
    }
    
    // Array specific methods
    size_t size() const {
        return values_.size();
    }
    
    void add(std::unique_ptr<Value> value) {
        values_.push_back(std::move(value));
    }
    
    const Value& at(size_t index) const {
        if (index >= values_.size()) {
            throw JsonException("Array index out of bounds");
        }
        return *values_[index];
    }
    
    Value& at(size_t index) {
        if (index >= values_.size()) {
            throw JsonException("Array index out of bounds");
        }
        return *values_[index];
    }
    
private:
    std::vector<std::unique_ptr<Value>> values_;
};

// Object type
class Object : public Value {
public:
    Object() = default;
    
    Object(std::initializer_list<std::pair<const std::string, std::unique_ptr<Value>>> init) {
        for (auto& pair : init) {
            if (pair.second) {
                values_[pair.first] = pair.second->clone();
            }
        }
    }
    
    Type type() const override { return Type::Object; }
    
    const Object& asObject() const override {
        return *this;
    }
    
    Object& asObject() override {
        return *this;
    }
    
    std::string toString() const override {
        std::string result{"{"};
        bool first = true;
        
        for (const auto& pair : values_) {
            if (!first) {
                result += ",";
            }
            first = false;
            
            result += "\"" + pair.first + "\":" + pair.second->toString();
        }
        
        result += "}";
        return result;
    }
    
    std::unique_ptr<Value> clone() const override {
        auto result = std::make_unique<Object>();
        for (const auto& pair : values_) {
            result->values_[pair.first] = pair.second->clone();
        }
        return result;
    }
    
    // Object specific methods
    bool has(const std::string& key) const {
        return values_.find(key) != values_.end();
    }
    
    void set(const std::string& key, std::unique_ptr<Value> value) {
        values_[key] = std::move(value);
    }
    
    const Value& get(const std::string& key) const {
        auto it = values_.find(key);
        if (it == values_.end()) {
            throw JsonException("Key not found: " + key);
        }
        return *it->second;
    }
    
    Value& get(const std::string& key) {
        auto it = values_.find(key);
        if (it == values_.end()) {
            throw JsonException("Key not found: " + key);
        }
        return *it->second;
    }
    
    std::vector<std::string> keys() const {
        std::vector<std::string> result{};
        for (const auto& pair : values_) {
            result.push_back(pair.first);
        }
        return result;
    }
    
private:
    std::unordered_map<std::string, std::unique_ptr<Value>> values_;
};

// JSON parser class
class Parser {
public:
    static std::unique_ptr<Value> parse(const std::string& json) {
        Parser parser(json);
        return parser.parseValue();
    }
    
private:
    Parser(const std::string& json) : json_(json), pos_(0) {}
    
    // Skip whitespace
    void skipWhitespace() {
        while (pos_ < json_.size() && std::isspace(json_[pos_])) {
            pos_++;
        }
    }
    
    // Parse a value (dispatch to correct parser)
    std::unique_ptr<Value> parseValue() {
        skipWhitespace();
        
        if (pos_ >= json_.size()) {
            throw JsonException("Unexpected end of input");
        }
        
        switch (json_[pos_]) {
            case 'n': return parseNull();
            case 't': case 'f': return parseBoolean();
            case '"': return parseString();
            case '[': return parseArray();
            case '{': return parseObject();
            default:
                if (json_[pos_] == '-' || std::isdigit(json_[pos_])) {
                    return parseNumber();
                }
                throw JsonException("Unexpected character: " + std::string(1, json_[pos_]));
        }
    }
    
    // Parse null
    std::unique_ptr<Value> parseNull() {
        if (pos_ + 3 >= json_.size() || 
            json_[pos_] != 'n' || 
            json_[pos_ + 1] != 'u' || 
            json_[pos_ + 2] != 'l' || 
            json_[pos_ + 3] != 'l') {
            throw JsonException("Expected 'null'");
        }
        
        pos_ += 4;
        return std::make_unique<Null>();
    }
    
    // Parse boolean
    std::unique_ptr<Value> parseBoolean() {
        if (pos_ + 3 < json_.size() && 
            json_[pos_] == 't' && 
            json_[pos_ + 1] == 'r' && 
            json_[pos_ + 2] == 'u' && 
            json_[pos_ + 3] == 'e') {
            
            pos_ += 4;
            return std::make_unique<Boolean>(true);
        }
        
        if (pos_ + 4 < json_.size() && 
            json_[pos_] == 'f' && 
            json_[pos_ + 1] == 'a' && 
            json_[pos_ + 2] == 'l' && 
            json_[pos_ + 3] == 's' && 
            json_[pos_ + 4] == 'e') {
            
            pos_ += 5;
            return std::make_unique<Boolean>(false);
        }
        
        throw JsonException("Expected 'true' or 'false'");
    }
    
    // Parse number
    std::unique_ptr<Value> parseNumber() {
        size_t start = pos_;
        
        // Handle negative sign
        if (json_[pos_] == '-') {
            pos_++;
        }
        
        // Handle integer part
        while (pos_ < json_.size() && std::isdigit(json_[pos_])) {
            pos_++;
        }
        
        // Handle decimal part
        if (pos_ < json_.size() && json_[pos_] == '.') {
            pos_++;
            
            // At least one digit required after decimal point
            if (pos_ >= json_.size() || !std::isdigit(json_[pos_])) {
                throw JsonException("Expected digit after decimal point");
            }
            
            while (pos_ < json_.size() && std::isdigit(json_[pos_])) {
                pos_++;
            }
        }
        
        // Handle exponent part
        if (pos_ < json_.size() && (json_[pos_] == 'e' || json_[pos_] == 'E')) {
            pos_++;
            
            // Handle exponent sign
            if (pos_ < json_.size() && (json_[pos_] == '+' || json_[pos_] == '-')) {
                pos_++;
            }
            
            // At least one digit required in exponent
            if (pos_ >= json_.size() || !std::isdigit(json_[pos_])) {
                throw JsonException("Expected digit in exponent");
            }
            
            while (pos_ < json_.size() && std::isdigit(json_[pos_])) {
                pos_++;
            }
        }
        
        std::string numStr = json_.substr(start, pos_ - start);
        return std::make_unique<Number>(std::stod(numStr));
    }
    
    // Parse string
    std::unique_ptr<Value> parseString() {
        if (json_[pos_] != '"') {
            throw JsonException("Expected '\"'");
        }
        
        pos_++; // Skip opening quote
        std::string result{};
        
        while (pos_ < json_.size() && json_[pos_] != '"') {
            if (json_[pos_] == '\\') {
                if (pos_ + 1 >= json_.size()) {
                    throw JsonException("Unexpected end of input in escape sequence");
                }
                
                pos_++; // Skip backslash
                switch (json_[pos_]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        if (pos_ + 4 >= json_.size()) {
                            throw JsonException("Unexpected end of input in Unicode escape sequence");
                        }
                        
                        std::string hexCode = json_.substr(pos_ + 1, 4);
                        for (char c : hexCode) {
                            if (!std::isxdigit(c)) {
                                throw JsonException("Invalid Unicode escape sequence");
                            }
                        }
                        
                        int codePoint = std::stoi(hexCode, nullptr, 16);
                        
                        // Handle UTF-16 surrogate pairs
                        if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                            // High surrogate - check for low surrogate
                            if (pos_ + 6 >= json_.size() || 
                                json_[pos_ + 5] != '\\' || 
                                json_[pos_ + 6] != 'u') {
                                throw JsonException("Expected low surrogate in Unicode pair");
                            }
                            
                            pos_ += 6; // Move to the 'u' in second \u sequence
                            
                            if (pos_ + 4 >= json_.size()) {
                                throw JsonException("Unexpected end of input in Unicode escape sequence");
                            }
                            
                            std::string hexCode2 = json_.substr(pos_ + 1, 4);
                            for (char c : hexCode2) {
                                if (!std::isxdigit(c)) {
                                    throw JsonException("Invalid Unicode escape sequence");
                                }
                            }
                            
                            int codePoint2 = std::stoi(hexCode2, nullptr, 16);
                            
                            if (codePoint2 < 0xDC00 || codePoint2 > 0xDFFF) {
                                throw JsonException("Invalid low surrogate in Unicode pair");
                            }
                            
                            // Decode surrogate pair
                            int utf32 = 0x10000 + ((codePoint - 0xD800) << 10) + (codePoint2 - 0xDC00);
                            
                            // Convert to UTF-8
                            if (utf32 < 0x10000) {
                                // Should never happen with valid surrogate pair
                                throw JsonException("Invalid Unicode surrogate pair");
                            } else {
                                result += static_cast<char>(0xF0 | ((utf32 >> 18) & 0x07));
                                result += static_cast<char>(0x80 | ((utf32 >> 12) & 0x3F));
                                result += static_cast<char>(0x80 | ((utf32 >> 6) & 0x3F));
                                result += static_cast<char>(0x80 | (utf32 & 0x3F));
                            }
                            
                            pos_ += 4; // Skip the second \u sequence
                        } else {
                            // Regular Unicode character
                            
                            // Convert to UTF-8
                            if (codePoint < 0x80) {
                                result += static_cast<char>(codePoint);
                            } else if (codePoint < 0x800) {
                                result += static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F));
                                result += static_cast<char>(0x80 | (codePoint & 0x3F));
                            } else {
                                result += static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F));
                                result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                                result += static_cast<char>(0x80 | (codePoint & 0x3F));
                            }
                            
                            pos_ += 4; // Skip the \u sequence
                        }
                        break;
                    }
                    default:
                        throw JsonException("Invalid escape sequence: \\" + std::string(1, json_[pos_]));
                }
            } else {
                result += json_[pos_];
            }
            
            pos_++;
        }
        
        if (pos_ >= json_.size()) {
            throw JsonException("Unterminated string");
        }
        
        pos_++; // Skip closing quote
        return std::make_unique<String>(std::move(result));
    }
    
    // Parse array
    std::unique_ptr<Value> parseArray() {
        if (json_[pos_] != '[') {
            throw JsonException("Expected '['");
        }
        
        pos_++; // Skip opening bracket
        auto array = std::make_unique<Array>();
        
        skipWhitespace();
        
        // Check for empty array
        if (pos_ < json_.size() && json_[pos_] == ']') {
            pos_++; // Skip closing bracket
            return array;
        }
        
        // Parse array elements
        while (true) {
            skipWhitespace();
            
            // Parse element
            array->add(parseValue());
            
            skipWhitespace();
            
            if (pos_ >= json_.size()) {
                throw JsonException("Unterminated array");
            }
            
            if (json_[pos_] == ']') {
                pos_++; // Skip closing bracket
                break;
            }
            
            if (json_[pos_] != ',') {
                throw JsonException("Expected ',' or ']'");
            }
            
            pos_++; // Skip comma
        }
        
        return array;
    }
    
    // Parse object
    std::unique_ptr<Value> parseObject() {
        if (json_[pos_] != '{') {
            throw JsonException("Expected '{'");
        }
        
        pos_++; // Skip opening brace
        auto object = std::make_unique<Object>();
        
        skipWhitespace();
        
        // Check for empty object
        if (pos_ < json_.size() && json_[pos_] == '}') {
            pos_++; // Skip closing brace
            return object;
        }
        
        // Parse object members
        while (true) {
            skipWhitespace();
            
            // Parse key
            if (pos_ >= json_.size() || json_[pos_] != '"') {
                throw JsonException("Expected string key");
            }
            
            std::unique_ptr<Value> keyValue = parseString();
            std::string key = keyValue->asString();
            
            skipWhitespace();
            
            if (pos_ >= json_.size() || json_[pos_] != ':') {
                throw JsonException("Expected ':'");
            }
            
            pos_++; // Skip colon
            
            // Parse value
            object->set(key, parseValue());
            
            skipWhitespace();
            
            if (pos_ >= json_.size()) {
                throw JsonException("Unterminated object");
            }
            
            if (json_[pos_] == '}') {
                pos_++; // Skip closing brace
                break;
            }
            
            if (json_[pos_] != ',') {
                throw JsonException("Expected ',' or '}'");
            }
            
            pos_++; // Skip comma
        }
        
        return object;
    }
    
    std::string json_;
    size_t pos_;
};

// Helper functions for JSON creation and parsing
namespace make {
    inline std::unique_ptr<Value> null() {
        return std::make_unique<Null>();
    }
    
    inline std::unique_ptr<Value> boolean(bool value) {
        return std::make_unique<Boolean>(value);
    }
    
    inline std::unique_ptr<Value> number(double value) {
        return std::make_unique<Number>(value);
    }
    
    inline std::unique_ptr<Value> string(const std::string& value) {
        return std::make_unique<String>(value);
    }
    
    inline std::unique_ptr<Value> string(std::string&& value) {
        return std::make_unique<String>(std::move(value));
    }
    
    inline std::unique_ptr<Value> array() {
        return std::make_unique<Array>();
    }
    
    template<typename... Args>
    inline std::unique_ptr<Array> array(Args&&... args) {
        auto arr = std::make_unique<Array>();
        (arr->add(std::forward<Args>(args)), ...);
        return arr;
    }
    
    inline std::unique_ptr<Value> object() {
        return std::make_unique<Object>();
    }
    
    // Helper for creating objects with initializer lists
    struct KeyValue {
        std::string key;
        std::unique_ptr<Value> value;
        
        KeyValue(std::string k, std::unique_ptr<Value> v)
            : key(std::move(k)), value(std::move(v)) {}
    };
    
    inline std::unique_ptr<Object> object(std::initializer_list<KeyValue> init) {
        auto obj = std::make_unique<Object>();
        for (const auto& kv : init) {
            obj->set(kv.key, std::move(const_cast<KeyValue&>(kv).value));
        }
        return obj;
    }
}

// Parse a JSON string
inline std::unique_ptr<Value> parse(const std::string& json) {
    return Parser::parse(json);
}

// Convert a JSON value to a string
inline std::string stringify(const Value& value) {
    return value.toString();
}

// Helper function to parse a JSON string
inline std::unique_ptr<Value> parse(const char* json) {
    return parse(std::string(json));
}

} // namespace json

#endif // JSON_HPP