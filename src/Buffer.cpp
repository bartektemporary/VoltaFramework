#include "Buffer.hpp"
#include "VoltaFramework.hpp" // For getFramework()
#include <cstring> // For std::memcpy

// Buffer class implementations
void Buffer::writeUInt8(uint8_t value, size_t offset) {
    if (offset < data.size()) data[offset] = value;
}

void Buffer::writeInt8(int8_t value, size_t offset) {
    writeUInt8(static_cast<uint8_t>(value), offset);
}

void Buffer::writeUInt16(uint16_t value, size_t offset) {
    if (offset + 1 < data.size()) {
        data[offset] = static_cast<uint8_t>(value & 0xFF);
        data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    }
}

void Buffer::writeInt16(int16_t value, size_t offset) {
    writeUInt16(static_cast<uint16_t>(value), offset);
}

void Buffer::writeUInt32(uint32_t value, size_t offset) {
    if (offset + 3 < data.size()) {
        data[offset] = static_cast<uint8_t>(value & 0xFF);
        data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        data[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        data[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
}

void Buffer::writeInt32(int32_t value, size_t offset) {
    writeUInt32(static_cast<uint32_t>(value), offset);
}

void Buffer::writeUInt64(uint64_t value, size_t offset) {
    if (offset + 7 < data.size()) {
        data[offset] = static_cast<uint8_t>(value & 0xFF);
        data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        data[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        data[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        data[offset + 4] = static_cast<uint8_t>((value >> 32) & 0xFF);
        data[offset + 5] = static_cast<uint8_t>((value >> 40) & 0xFF);
        data[offset + 6] = static_cast<uint8_t>((value >> 48) & 0xFF);
        data[offset + 7] = static_cast<uint8_t>((value >> 56) & 0xFF);
    }
}

void Buffer::writeInt64(int64_t value, size_t offset) {
    writeUInt64(static_cast<uint64_t>(value), offset);
}

void Buffer::writeString(const std::string& value, size_t offset) {
    if (offset + value.size() <= data.size()) {
        std::memcpy(data.data() + offset, value.data(), value.size());
    }
}

uint8_t Buffer::readUInt8(size_t offset) const {
    return (offset < data.size()) ? data[offset] : 0;
}

int8_t Buffer::readInt8(size_t offset) const {
    return static_cast<int8_t>(readUInt8(offset));
}

uint16_t Buffer::readUInt16(size_t offset) const {
    if (offset + 1 < data.size()) {
        return static_cast<uint16_t>(data[offset]) |
               (static_cast<uint16_t>(data[offset + 1]) << 8);
    }
    return 0;
}

int16_t Buffer::readInt16(size_t offset) const {
    return static_cast<int16_t>(readUInt16(offset));
}

uint32_t Buffer::readUInt32(size_t offset) const {
    if (offset + 3 < data.size()) {
        return static_cast<uint32_t>(data[offset]) |
               (static_cast<uint32_t>(data[offset + 1]) << 8) |
               (static_cast<uint32_t>(data[offset + 2]) << 16) |
               (static_cast<uint32_t>(data[offset + 3]) << 24);
    }
    return 0;
}

int32_t Buffer::readInt32(size_t offset) const {
    return static_cast<int32_t>(readUInt32(offset));
}

uint64_t Buffer::readUInt64(size_t offset) const {
    if (offset + 7 < data.size()) {
        return static_cast<uint64_t>(data[offset]) |
               (static_cast<uint64_t>(data[offset + 1]) << 8) |
               (static_cast<uint64_t>(data[offset + 2]) << 16) |
               (static_cast<uint64_t>(data[offset + 3]) << 24) |
               (static_cast<uint64_t>(data[offset + 4]) << 32) |
               (static_cast<uint64_t>(data[offset + 5]) << 40) |
               (static_cast<uint64_t>(data[offset + 6]) << 48) |
               (static_cast<uint64_t>(data[offset + 7]) << 56);
    }
    return 0;
}

int64_t Buffer::readInt64(size_t offset) const {
    return static_cast<int64_t>(readUInt64(offset));
}

std::string Buffer::readString(size_t offset, size_t length) const {
    if (offset + length <= data.size()) {
        return {data.begin() + offset, data.begin() + offset + length};
    }
    return {};
}

// Lua functions
int l_buffer_alloc(lua_State* L) {
    size_t size{static_cast<size_t>(luaL_checkinteger(L, 1))};
    if (size <= 0) {
        luaL_error(L, "Buffer size must be positive");
        return 0;
    }

    VoltaFramework* framework{getFramework(L)};
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }

    Buffer* buffer{new Buffer(size)};
    framework->bufferCache[buffer] = std::unique_ptr<Buffer>{buffer};

    Buffer** ud{static_cast<Buffer**>(lua_newuserdata(L, sizeof(Buffer*)))};
    *ud = buffer;
    luaL_getmetatable(L, "Buffer");
    lua_setmetatable(L, -2);

    return 1;
}

#define BUFFER_CHECK_ARGS \
    Buffer** ud = static_cast<Buffer**>(luaL_checkudata(L, 1, "Buffer")); \
    Buffer* buffer = *ud; \
    size_t offset = luaL_checkinteger(L, 3); \
    if (offset >= buffer->getSize()) { \
        luaL_error(L, "Offset out of bounds"); \
        return 0; \
    }

int l_buffer_writeUInt8(lua_State* L) {
    BUFFER_CHECK_ARGS
    uint8_t value{static_cast<uint8_t>(luaL_checkinteger(L, 2))};
    buffer->writeUInt8(value, offset);
    return 0;
}

int l_buffer_readUInt8(lua_State* L) {
    BUFFER_CHECK_ARGS
    lua_pushinteger(L, buffer->readUInt8(offset));
    return 1;
}

int l_buffer_writeInt8(lua_State* L) {
    BUFFER_CHECK_ARGS
    int8_t value{static_cast<int8_t>(luaL_checkinteger(L, 2))};
    buffer->writeInt8(value, offset);
    return 0;
}

int l_buffer_readInt8(lua_State* L) {
    BUFFER_CHECK_ARGS
    lua_pushinteger(L, buffer->readInt8(offset));
    return 1;
}

int l_buffer_writeUInt16(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 1 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 16-bit write");
        return 0;
    }
    uint16_t value{static_cast<uint16_t>(luaL_checkinteger(L, 2))};
    buffer->writeUInt16(value, offset);
    return 0;
}

int l_buffer_readUInt16(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 1 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 16-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readUInt16(offset));
    return 1;
}

int l_buffer_writeInt16(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 1 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 16-bit write");
        return 0;
    }
    int16_t value{static_cast<int16_t>(luaL_checkinteger(L, 2))};
    buffer->writeInt16(value, offset);
    return 0;
}

int l_buffer_readInt16(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 1 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 16-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readInt16(offset));
    return 1;
}

int l_buffer_writeUInt32(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 3 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 32-bit write");
        return 0;
    }
    uint32_t value{static_cast<uint32_t>(luaL_checkinteger(L, 2))};
    buffer->writeUInt32(value, offset);
    return 0;
}

int l_buffer_readUInt32(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 3 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 32-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readUInt32(offset));
    return 1;
}

int l_buffer_writeInt32(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 3 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 32-bit write");
        return 0;
    }
    int32_t value{static_cast<int32_t>(luaL_checkinteger(L, 2))};
    buffer->writeInt32(value, offset);
    return 0;
}

int l_buffer_readInt32(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 3 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 32-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readInt32(offset));
    return 1;
}

int l_buffer_writeUInt64(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 7 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 64-bit write");
        return 0;
    }
    uint64_t value{static_cast<uint64_t>(luaL_checkinteger(L, 2))};
    buffer->writeUInt64(value, offset);
    return 0;
}

int l_buffer_readUInt64(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 7 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 64-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readUInt64(offset));
    return 1;
}

int l_buffer_writeInt64(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 7 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 64-bit write");
        return 0;
    }
    int64_t value{static_cast<int64_t>(luaL_checkinteger(L, 2))};
    buffer->writeInt64(value, offset);
    return 0;
}

int l_buffer_readInt64(lua_State* L) {
    BUFFER_CHECK_ARGS
    if (offset + 7 >= buffer->getSize()) {
        luaL_error(L, "Offset out of bounds for 64-bit read");
        return 0;
    }
    lua_pushinteger(L, buffer->readInt64(offset));
    return 1;
}

int l_buffer_writeString(lua_State* L) {
    Buffer** ud{static_cast<Buffer**>(luaL_checkudata(L, 1, "Buffer"))};
    Buffer* buffer{*ud};
    const char* value{luaL_checkstring(L, 2)};
    size_t offset{static_cast<size_t>(luaL_checkinteger(L, 3))};

    size_t len{std::strlen(value)};
    if (offset + len > buffer->getSize()) {
        luaL_error(L, "String write exceeds buffer size");
        return 0;
    }

    buffer->writeString({value}, offset);
    return 0;
}

int l_buffer_readString(lua_State* L) {
    Buffer** ud{static_cast<Buffer**>(luaL_checkudata(L, 1, "Buffer"))};
    Buffer* buffer{*ud};
    size_t offset{static_cast<size_t>(luaL_checkinteger(L, 2))};
    size_t length{static_cast<size_t>(luaL_checkinteger(L, 3))};

    if (offset + length > buffer->getSize()) {
        luaL_error(L, "String read exceeds buffer size");
        return 0;
    }

    std::string result{buffer->readString(offset, length)};
    lua_pushstring(L, result.c_str());
    return 1;
}

int l_buffer_size(lua_State* L) {
    Buffer** ud{static_cast<Buffer**>(luaL_checkudata(L, 1, "Buffer"))};
    Buffer* buffer{*ud};
    lua_pushinteger(L, buffer->getSize());
    return 1;
}