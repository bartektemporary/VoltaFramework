#include "VoltaFramework.hpp"

// Existing function: Check if a path exists
int l_filesystem_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool exists = fs::exists(path);
    lua_pushboolean(L, exists);
    return 1;
}

// Existing function: Check if a path is a directory
int l_filesystem_isDirectory(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isDir = fs::exists(path) && fs::is_directory(path);
    lua_pushboolean(L, isDir);
    return 1;
}

// Existing function: Create a directory
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

// Existing function: Remove a file or directory
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

// Existing function: List directory contents
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

// Existing function: Get current working directory
int l_filesystem_getWorkingDir(lua_State* L) {
    std::string cwd = fs::current_path().string();
    lua_pushstring(L, cwd.c_str());
    return 1;
}

// Existing function: Set current working directory
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

// Existing function: Get file size
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

// New function: Check if a path is a regular file
int l_filesystem_isRegularFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isFile = fs::exists(path) && fs::is_regular_file(path);
    lua_pushboolean(L, isFile);
    return 1;
}

// New function: Check if a path is a character device
int l_filesystem_isCharacterFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isChar = fs::exists(path) && fs::is_character_file(path);
    lua_pushboolean(L, isChar);
    return 1;
}

// New function: Check if a path is a FIFO (named pipe)
int l_filesystem_isFifo(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isFifo = fs::exists(path) && fs::is_fifo(path);
    lua_pushboolean(L, isFifo);
    return 1;
}

// New function: Check if a path is a socket
int l_filesystem_isSocket(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isSocket = fs::exists(path) && fs::is_socket(path);
    lua_pushboolean(L, isSocket);
    return 1;
}

// New function: Get the canonical path (absolute path)
int l_filesystem_canonical(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    try {
        std::string canonicalPath = fs::canonical(path).string();
        lua_pushstring(L, canonicalPath.c_str());
    } catch (const fs::filesystem_error&) {
        lua_pushnil(L);
    }
    return 1;
}

// New function: Get the relative path from one path to another
int l_filesystem_relative(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* base = luaL_checkstring(L, 2);
    try {
        std::string relPath = fs::relative(path, base).string();
        lua_pushstring(L, relPath.c_str());
    } catch (const fs::filesystem_error&) {
        lua_pushnil(L);
    }
    return 1;
}

// New function: Copy a file or directory
int l_filesystem_copy(lua_State* L) {
    const char* from = luaL_checkstring(L, 1);
    const char* to = luaL_checkstring(L, 2);
    bool recursive = lua_toboolean(L, 3); // Optional third argument for recursive copying
    try {
        if (recursive) {
            fs::copy(from, to, fs::copy_options::recursive);
        } else {
            fs::copy(from, to);
        }
        lua_pushboolean(L, true);
    } catch (const fs::filesystem_error&) {
        lua_pushboolean(L, false);
    }
    return 1;
}

// New function: Rename a file or directory
int l_filesystem_rename(lua_State* L) {
    const char* oldPath = luaL_checkstring(L, 1);
    const char* newPath = luaL_checkstring(L, 2);
    try {
        fs::rename(oldPath, newPath);
        lua_pushboolean(L, true);
    } catch (const fs::filesystem_error&) {
        lua_pushboolean(L, false);
    }
    return 1;
}

// New function: Get the last write time of a file
int l_filesystem_lastWriteTime(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    try {
        auto time = fs::last_write_time(path);
        auto duration = time.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        lua_pushnumber(L, static_cast<double>(seconds));
    } catch (const fs::filesystem_error&) {
        lua_pushnil(L);
    }
    return 1;
}

// New function: Check if a path is empty
int l_filesystem_isEmpty(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool isEmpty = fs::is_empty(path);
    lua_pushboolean(L, isEmpty);
    return 1;
}

// New function: Get the absolute path
int l_filesystem_absolute(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    try {
        std::string absPath = fs::absolute(path).string();
        lua_pushstring(L, absPath.c_str());
    } catch (const fs::filesystem_error&) {
        lua_pushnil(L);
    }
    return 1;
}

// New function: Get the parent path
int l_filesystem_parentPath(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::string parent = fs::path(path).parent_path().string();
    lua_pushstring(L, parent.c_str());
    return 1;
}

// New function: Get the filename
int l_filesystem_filename(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::string filename = fs::path(path).filename().string();
    lua_pushstring(L, filename.c_str());
    return 1;
}

// New function: Get the stem (filename without extension)
int l_filesystem_stem(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::string stem = fs::path(path).stem().string();
    lua_pushstring(L, stem.c_str());
    return 1;
}

// New function: Get the extension
int l_filesystem_extension(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::string ext = fs::path(path).extension().string();
    lua_pushstring(L, ext.c_str());
    return 1;
}

// New function: Get the temp directory path
int l_filesystem_tempDirectory(lua_State* L) {
    std::string tempDir = fs::temp_directory_path().string();
    lua_pushstring(L, tempDir.c_str());
    return 1;
}

// New function: Get the equivalent path (check if two paths point to the same file)
int l_filesystem_equivalent(lua_State* L) {
    const char* path1 = luaL_checkstring(L, 1);
    const char* path2 = luaL_checkstring(L, 2);
    try {
        bool equiv = fs::equivalent(path1, path2);
        lua_pushboolean(L, equiv);
    } catch (const fs::filesystem_error&) {
        lua_pushboolean(L, false);
    }
    return 1;
}

int l_filesystem_getUserDir(lua_State* L) {
    std::string userDir;

    try {
#ifdef _WIN32
        // On Windows, use USERPROFILE environment variable
        const char* userProfile = std::getenv("USERPROFILE");
        if (userProfile) {
            userDir = userProfile;
        } else {
            // Fallback: Try HOMEDRIVE + HOMEPATH
            const char* homeDrive = std::getenv("HOMEDRIVE");
            const char* homePath = std::getenv("HOMEPATH");
            if (homeDrive && homePath) {
                userDir = std::string(homeDrive) + homePath;
            } else {
                // If all fails, throw an error
                throw std::runtime_error("Could not determine user directory on Windows");
            }
        }
#else
        // On Unix-like systems (Linux, macOS), use HOME environment variable
        const char* home = std::getenv("HOME");
        if (home) {
            userDir = home;
        } else {
            // Fallback: Try getpwuid for the current user
            #include <pwd.h>
            #include <unistd.h>
            struct passwd* pw = getpwuid(getuid());
            if (pw && pw->pw_dir) {
                userDir = pw->pw_dir;
            } else {
                throw std::runtime_error("Could not determine user directory on Unix-like system");
            }
        }
#endif
        // Ensure the path is valid and normalized using std::filesystem
        fs::path userPath(userDir);
        userDir = fs::absolute(userPath).string();
        lua_pushstring(L, userDir.c_str());
    } catch (const std::exception& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2; // Return nil and an error message
    }

    return 1;
}