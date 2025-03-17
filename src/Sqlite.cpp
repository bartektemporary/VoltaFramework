#include "VoltaFramework.hpp"

// Helper function to get framework (assumed to exist)
extern VoltaFramework* getFramework(lua_State* L);

int l_sqlite_open(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1); // Ensures arg 1 is a string
    VoltaFramework* framework = getFramework(L);

    sqlite3* db;
    int rc = sqlite3_open(filename, &db);

    if (rc != SQLITE_OK) {
        lua_pushnil(L);
        lua_pushstring(L, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 2;
    }

    std::string dbKey = std::string(filename) + "_" + std::to_string(reinterpret_cast<uintptr_t>(db));
    framework->databaseCache[dbKey] = db;

    lua_pushlightuserdata(L, db);
    return 1;
}

int l_sqlite_close(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }

    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    VoltaFramework* framework = getFramework(L);

    if (!db) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid database handle (null)");
        return 2;
    }

    // Validate that the pointer exists in the cache
    auto it = std::find_if(framework->databaseCache.begin(), framework->databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == framework->databaseCache.end()) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Unknown or already closed database handle");
        return 2;
    }

    framework->databaseCache.erase(it);
    int rc = sqlite3_close(db); // Fixed: rc is now declared
    lua_pushboolean(L, rc == SQLITE_OK);
    if (rc != SQLITE_OK) {
        lua_pushstring(L, sqlite3_errmsg(db));
        return 2;
    }
    return 1;
}

int l_sqlite_exec(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2); // Ensures arg 2 is a string

    VoltaFramework* framework = getFramework(L);
    auto it = std::find_if(framework->databaseCache.begin(), framework->databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == framework->databaseCache.end()) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid or closed database handle");
        return 2;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, errMsg ? errMsg : "Unknown SQLite error");
        sqlite3_free(errMsg);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

int l_sqlite_prepare(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2);

    VoltaFramework* framework = getFramework(L);
    auto it = std::find_if(framework->databaseCache.begin(), framework->databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == framework->databaseCache.end()) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid or closed database handle");
        return 2;
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        lua_pushnil(L);
        lua_pushstring(L, sqlite3_errmsg(db));
        return 2;
    }

    lua_pushlightuserdata(L, stmt);
    return 1;
}

int l_sqlite_step(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    if (!stmt) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid statement handle (null)");
        return 2;
    }

    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        lua_pushboolean(L, 1); // true for row available
    } else if (rc == SQLITE_DONE) {
        lua_pushboolean(L, 0); // false for complete
    } else {
        lua_pushnil(L);
        lua_pushstring(L, sqlite3_errmsg(sqlite3_db_handle(stmt)));
        return 2;
    }

    return 1;
}

int l_sqlite_finalize(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    if (!stmt) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid statement handle (null)");
        return 2;
    }

    int rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to finalize statement");
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

int l_sqlite_bind(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    if (!stmt) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid statement handle (null)");
        return 2;
    }
    int index = luaL_checkinteger(L, 2);

    int type = lua_type(L, 3);
    int rc;

    switch (type) {
        case LUA_TNIL:
            rc = sqlite3_bind_null(stmt, index);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, 3)) {
                rc = sqlite3_bind_int64(stmt, index, lua_tointeger(L, 3));
            } else {
                rc = sqlite3_bind_double(stmt, index, lua_tonumber(L, 3));
            }
            break;
        case LUA_TSTRING: {
            size_t len;
            const char* data = lua_tolstring(L, 3, &len);
            rc = sqlite3_bind_blob(stmt, index, data, len, SQLITE_TRANSIENT);
            break;
        }
        default:
            lua_pushboolean(L, 0);
            lua_pushstring(L, "Unsupported bind type");
            return 2;
    }

    if (rc != SQLITE_OK) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, sqlite3_errmsg(sqlite3_db_handle(stmt)));
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

int l_sqlite_column(lua_State* L) {
    if (!lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    if (!stmt) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid statement handle (null)");
        return 2;
    }
    int index = luaL_checkinteger(L, 2);

    int type = sqlite3_column_type(stmt, index);

    switch (type) {
        case SQLITE_INTEGER:
            lua_pushinteger(L, sqlite3_column_int64(stmt, index));
            break;
        case SQLITE_FLOAT:
            lua_pushnumber(L, sqlite3_column_double(stmt, index));
            break;
        case SQLITE_TEXT:
            lua_pushstring(L, reinterpret_cast<const char*>(sqlite3_column_text(stmt, index)));
            break;
        case SQLITE_BLOB: {
            const void* blob = sqlite3_column_blob(stmt, index);
            int bytes = sqlite3_column_bytes(stmt, index);
            lua_pushlstring(L, static_cast<const char*>(blob), bytes);
            break;
        }
        case SQLITE_NULL:
            lua_pushnil(L);
            break;
        default:
            lua_pushnil(L);
            lua_pushstring(L, "Unknown column type");
            return 2;
    }

    return 1;
}