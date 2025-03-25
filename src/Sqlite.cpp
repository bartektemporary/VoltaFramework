#include "VoltaFramework.hpp"

// Helper function to get framework (assumed to exist)
extern VoltaFramework* getFramework(lua_State* L);

sqlite3* VoltaFramework::openDatabase(const std::string& filename, std::string& errorMsg) {
    sqlite3* db;
    int rc = sqlite3_open(filename.c_str(), &db);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_close(db);
        return nullptr;
    }
    std::string dbKey = filename + "_" + std::to_string(reinterpret_cast<uintptr_t>(db));
    databaseCache[dbKey] = db;
    return db;
}

bool VoltaFramework::closeDatabase(sqlite3* db, std::string& errorMsg) {
    if (!db) {
        errorMsg = "Invalid database handle (null)";
        return false;
    }

    auto it = std::find_if(databaseCache.begin(), databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == databaseCache.end()) {
        errorMsg = "Unknown or already closed database handle";
        return false;
    }

    int rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        return false;
    }

    databaseCache.erase(it);
    return true;
}

bool VoltaFramework::executeSQL(sqlite3* db, const std::string& sql, std::string& errorMsg) {
    if (!db) {
        errorMsg = "Invalid database handle (null)";
        return false;
    }

    auto it = std::find_if(databaseCache.begin(), databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == databaseCache.end()) {
        errorMsg = "Invalid or closed database handle";
        return false;
    }

    char* errMsgC = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsgC);
    if (rc != SQLITE_OK) {
        errorMsg = errMsgC ? errMsgC : "Unknown SQLite error";
        sqlite3_free(errMsgC);
        return false;
    }
    return true;
}

sqlite3_stmt* VoltaFramework::prepareStatement(sqlite3* db, const std::string& sql, std::string& errorMsg) {
    if (!db) {
        errorMsg = "Invalid database handle (null)";
        return nullptr;
    }

    auto it = std::find_if(databaseCache.begin(), databaseCache.end(),
        [db](const auto& pair) { return pair.second == db; });
    if (it == databaseCache.end()) {
        errorMsg = "Invalid or closed database handle";
        return nullptr;
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        return nullptr;
    }
    return stmt;
}

int VoltaFramework::stepStatement(sqlite3_stmt* stmt, std::string& errorMsg) {
    if (!stmt) {
        errorMsg = "Invalid statement handle (null)";
        return SQLITE_ERROR;
    }
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        errorMsg = sqlite3_errmsg(sqlite3_db_handle(stmt));
    }
    return rc;
}

bool VoltaFramework::finalizeStatement(sqlite3_stmt* stmt, std::string& errorMsg) {
    if (!stmt) {
        errorMsg = "Invalid statement handle (null)";
        return false;
    }
    int rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        errorMsg = "Failed to finalize statement";
        return false;
    }
    return true;
}

bool VoltaFramework::bindValue(sqlite3_stmt* stmt, int index, const std::variant<std::monostate, int64_t, double, std::string>& value, std::string& errorMsg) {
    if (!stmt) {
        errorMsg = "Invalid statement handle (null)";
        return false;
    }

    int rc;
    if (std::holds_alternative<std::monostate>(value)) {
        rc = sqlite3_bind_null(stmt, index);
    } else if (std::holds_alternative<int64_t>(value)) {
        rc = sqlite3_bind_int64(stmt, index, std::get<int64_t>(value));
    } else if (std::holds_alternative<double>(value)) {
        rc = sqlite3_bind_double(stmt, index, std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        const std::string& str = std::get<std::string>(value);
        rc = sqlite3_bind_blob(stmt, index, str.data(), str.size(), SQLITE_TRANSIENT);
    } else {
        errorMsg = "Unsupported bind type";
        return false;
    }

    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(sqlite3_db_handle(stmt));
        return false;
    }
    return true;
}

std::variant<std::monostate, int64_t, double, std::string> VoltaFramework::getColumnValue(sqlite3_stmt* stmt, int index, std::string& errorMsg) {
    if (!stmt) {
        errorMsg = "Invalid statement handle (null)";
        return std::monostate{};
    }

    int type = sqlite3_column_type(stmt, index);
    switch (type) {
        case SQLITE_INTEGER:
            return sqlite3_column_int64(stmt, index);
        case SQLITE_FLOAT:
            return sqlite3_column_double(stmt, index);
        case SQLITE_TEXT:
            return std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, index)));
        case SQLITE_BLOB: {
            const void* blob = sqlite3_column_blob(stmt, index);
            int bytes = sqlite3_column_bytes(stmt, index);
            return std::string(static_cast<const char*>(blob), bytes);
        }
        case SQLITE_NULL:
            return std::monostate{};
        default:
            errorMsg = "Unknown column type";
            return std::monostate{};
    }
}

int l_sqlite_open(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushnil(L);
        lua_pushstring(L, "Framework is null");
        return 2;
    }
    const char* filename = luaL_checkstring(L, 1);
    std::string errorMsg;
    sqlite3* db = framework->openDatabase(filename, errorMsg);
    if (!db) {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    lua_pushlightuserdata(L, db);
    return 1;
}

int l_sqlite_close(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    std::string errorMsg;
    bool success = framework->closeDatabase(db, errorMsg);
    lua_pushboolean(L, success);
    if (!success) {
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    return 1;
}

int l_sqlite_exec(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2);
    std::string errorMsg;
    bool success = framework->executeSQL(db, sql, errorMsg);
    lua_pushboolean(L, success);
    if (!success) {
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    return 1;
}

int l_sqlite_prepare(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as database handle");
        return 2;
    }
    sqlite3* db = static_cast<sqlite3*>(lua_touserdata(L, 1));
    const char* sql = luaL_checkstring(L, 2);
    std::string errorMsg;
    sqlite3_stmt* stmt = framework->prepareStatement(db, sql, errorMsg);
    if (!stmt) {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    lua_pushlightuserdata(L, stmt);
    return 1;
}

int l_sqlite_step(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    std::string errorMsg;
    int rc = framework->stepStatement(stmt, errorMsg);
    if (rc == SQLITE_ROW) {
        lua_pushboolean(L, 1); // true for row available
    } else if (rc == SQLITE_DONE) {
        lua_pushboolean(L, 0); // false for complete
    } else {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    return 1;
}

int l_sqlite_finalize(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    std::string errorMsg;
    bool success = framework->finalizeStatement(stmt, errorMsg);
    lua_pushboolean(L, success);
    if (!success) {
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    return 1;
}

int l_sqlite_bind(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    int index = luaL_checkinteger(L, 2);
    std::variant<std::monostate, int64_t, double, std::string> value;
    int type = lua_type(L, 3);

    switch (type) {
        case LUA_TNIL:
            value = std::monostate{};
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, 3)) {
                value = lua_tointeger(L, 3);
            } else {
                value = lua_tonumber(L, 3);
            }
            break;
        case LUA_TSTRING: {
            size_t len;
            const char* data = lua_tolstring(L, 3, &len);
            value = std::string(data, len);
            break;
        }
        default:
            lua_pushboolean(L, 0);
            lua_pushstring(L, "Unsupported bind type");
            return 2;
    }

    std::string errorMsg;
    bool success = framework->bindValue(stmt, index, value, errorMsg);
    lua_pushboolean(L, success);
    if (!success) {
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }
    return 1;
}

int l_sqlite_column(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !lua_isuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Expected userdata as statement handle");
        return 2;
    }
    sqlite3_stmt* stmt = static_cast<sqlite3_stmt*>(lua_touserdata(L, 1));
    int index = luaL_checkinteger(L, 2);
    std::string errorMsg;
    auto value = framework->getColumnValue(stmt, index, errorMsg);
    if (!errorMsg.empty()) {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg.c_str());
        return 2;
    }

    if (std::holds_alternative<std::monostate>(value)) {
        lua_pushnil(L);
    } else if (std::holds_alternative<int64_t>(value)) {
        lua_pushinteger(L, std::get<int64_t>(value));
    } else if (std::holds_alternative<double>(value)) {
        lua_pushnumber(L, std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        const std::string& str = std::get<std::string>(value);
        lua_pushlstring(L, str.c_str(), str.size());
    }
    return 1;
}