#include "VoltaFramework.hpp"

int l_sqlite_open(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
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
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    VoltaFramework* framework = getFramework(L);
    
    if (db) {
        // Find and remove from cache
        for (auto it = framework->databaseCache.begin(); it != framework->databaseCache.end(); ++it) {
            if (it->second == db) {
                framework->databaseCache.erase(it);
                break;
            }
        }
        
        sqlite3_close(db);
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

int l_sqlite_exec(lua_State* L) {
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2);
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, errMsg);
        sqlite3_free(errMsg);
        return 2;
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

int l_sqlite_prepare(lua_State* L) {
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2);
    
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
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    
    int rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        lua_pushboolean(L, 1);  // true for row available
    } else if (rc == SQLITE_DONE) {
        lua_pushboolean(L, 0);  // false for complete
    } else {
        lua_pushnil(L);
        lua_pushstring(L, sqlite3_errmsg(sqlite3_db_handle(stmt)));
        return 2;
    }
    
    return 1;
}

int l_sqlite_finalize(lua_State* L) {
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    
    int rc = sqlite3_finalize(stmt);
    lua_pushboolean(L, rc == SQLITE_OK);
    return 1;
}

int l_sqlite_bind(lua_State* L) {
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
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
        case LUA_TSTRING:
            rc = sqlite3_bind_text(stmt, index, lua_tostring(L, 3), -1, SQLITE_TRANSIENT);
            break;
        default:
            luaL_error(L, "Unsupported bind type");
            return 0;
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
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
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
        case SQLITE_NULL:
            lua_pushnil(L);
            break;
        default:
            lua_pushnil(L);
            break;
    }
    
    return 1;
}