#include "VoltaFramework.hpp"

// Shallow copy: Copies only the top-level key-value pairs
int l_table_shallowCopy(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    
    lua_newtable(L); // Create new table for the copy
    
    // Iterate over the source table
    lua_pushnil(L); // First key
    while (lua_next(L, 1)) {
        // Stack: source table, key, value
        lua_pushvalue(L, -2); // Duplicate key
        lua_pushvalue(L, -2); // Duplicate value
        lua_settable(L, 2);   // Set in new table
        lua_pop(L, 1);        // Remove value, keep key for next iteration
    }
    
    return 1; // Return the new table
}