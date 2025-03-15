#include "VoltaFramework.hpp"

int l_filesystem_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool exists = fs::exists(path);
    lua_pushboolean(L, exists);
    return 1;
}

int l_filesystem_isDirectory(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isDir = fs::exists(path) && fs::is_directory(path);
    lua_pushboolean(L, isDir);
    return 1;
}

int l_filesystem_createDirectory(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool recursive = lua_toboolean(L, 2); // Optional second argument for recursive creation
    bool success;
    if (recursive) {
        success = fs::create_directories(path);
    } else {
        success = fs::create_directory(path);
    }
    lua_pushboolean(L, success);
    return 1;
}

int l_filesystem_remove(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool recursive = lua_toboolean(L, 2); // Optional second argument for recursive removal
    bool success;
    if (recursive) {
        success = fs::remove_all(path) > 0;
    } else {
        success = fs::remove(path);
    }
    lua_pushboolean(L, success);
    return 1;
}

int l_filesystem_listDir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    lua_newtable(L);
    int index = 1;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            lua_pushnumber(L, index++);
            lua_pushstring(L, entry.path().filename().string().c_str());
            lua_settable(L, -3);
        }
    } catch (const fs::filesystem_error&) {
        // Return empty table if directory doesn't exist or can't be accessed
    }
    
    return 1;
}

int l_filesystem_getWorkingDir(lua_State* L) {
    std::string cwd = fs::current_path().string();
    lua_pushstring(L, cwd.c_str());
    return 1;
}

int l_filesystem_setWorkingDir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    try {
        fs::current_path(path);
        lua_pushboolean(L, true);
    } catch (const fs::filesystem_error&) {
        lua_pushboolean(L, false);
    }
    return 1;
}

int l_filesystem_getFileSize(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    if (fs::exists(path) && fs::is_regular_file(path)) {
        uint64_t size = fs::file_size(path);
        lua_pushnumber(L, static_cast<double>(size));
    } else {
        lua_pushnumber(L, -1); // Return -1 if file doesn't exist or isn't a regular file
    }
    return 1;
}