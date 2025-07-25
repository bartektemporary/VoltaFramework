#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <lua.hpp>

class Buffer {
public:
    static constexpr size_t MAX_SIZE = 1'073'741'824; // 1 GiB in bytes
    Buffer(size_t size) {
        if (size > MAX_SIZE) {
            throw std::length_error("Buffer size exceeds maximum of 1 GiB");
        }
        data = std::vector<uint8_t>(size, 0);
    }
    
    uint8_t* getData() { return data.data(); }
    size_t getSize() const { return data.size(); }
    
    void resize(size_t newSize) {
        if (newSize > MAX_SIZE) {
            throw std::length_error("Buffer size exceeds maximum of 1 GiB");
        }
        data.resize(newSize, 0);
    }

    static Buffer fromString(const std::string& str) {
        if (str.size() > MAX_SIZE) {
            throw std::length_error("String size exceeds maximum buffer size of 1 GiB");
        }
        Buffer buffer{str.size()};
        buffer.writeString(str, 0);
        return buffer;
    }

    std::string toString() const {
        return readString(0, data.size());
    }

    // Write functions
    void writeUInt8(uint8_t value, size_t offset);
    void writeInt8(int8_t value, size_t offset);
    void writeUInt16(uint16_t value, size_t offset);
    void writeInt16(int16_t value, size_t offset);
    void writeUInt32(uint32_t value, size_t offset);
    void writeInt32(int32_t value, size_t offset);
    void writeUInt64(uint64_t value, size_t offset);
    void writeInt64(int64_t value, size_t offset);
    void writeString(const std::string& value, size_t offset);

    // Read functions
    uint8_t readUInt8(size_t offset) const;
    int8_t readInt8(size_t offset) const;
    uint16_t readUInt16(size_t offset) const;
    int16_t readInt16(size_t offset) const;
    uint32_t readUInt32(size_t offset) const;
    int32_t readInt32(size_t offset) const;
    uint64_t readUInt64(size_t offset) const;
    int64_t readInt64(size_t offset) const;
    std::string readString(size_t offset, size_t length) const;

private:
    std::vector<uint8_t> data;
};

// Lua functions
int l_buffer_alloc(lua_State* L);
int l_buffer_writeUInt8(lua_State* L);
int l_buffer_readUInt8(lua_State* L);
int l_buffer_writeInt8(lua_State* L);
int l_buffer_readInt8(lua_State* L);
int l_buffer_writeUInt16(lua_State* L);
int l_buffer_readUInt16(lua_State* L);
int l_buffer_writeInt16(lua_State* L);
int l_buffer_readInt16(lua_State* L);
int l_buffer_writeUInt32(lua_State* L);
int l_buffer_readUInt32(lua_State* L);
int l_buffer_writeInt32(lua_State* L);
int l_buffer_readInt32(lua_State* L);
int l_buffer_writeUInt64(lua_State* L);
int l_buffer_readUInt64(lua_State* L);
int l_buffer_writeInt64(lua_State* L);
int l_buffer_readInt64(lua_State* L);
int l_buffer_writeString(lua_State* L);
int l_buffer_readString(lua_State* L);
int l_buffer_fromString(lua_State* L);
int l_buffer_toString(lua_State* L);
int l_buffer_size(lua_State* L);

#endif // BUFFER_H