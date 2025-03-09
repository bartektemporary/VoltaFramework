#include "VoltaFramework.hpp"

int l_json_decode(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        luaL_error(L, "Expected string argument for json.decode");
        return 0;
    }

    const char* jsonStr = lua_tostring(L, 1);
    try {
        std::unique_ptr<json::Value> value = json::parse(jsonStr);
        VoltaFramework* framework = getFramework(L);
        if (!framework) {
            luaL_error(L, "Framework instance not found");
            return 0;
        }
        framework->jsonToLua(L, *value);
        return 1;
    } catch (const json::JsonException& e) {
        luaL_error(L, "JSON decode error: %s", e.what());
        return 0;
    }
}

int l_json_encode(lua_State* L) {
    std::cout << "Starting json.encode\n";
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }

    std::cout << "Converting Lua to JSON\n";
    std::unique_ptr<json::Value> value(framework->luaToJson(L, 1));
    std::cout << "Stringifying JSON\n";
    std::string jsonStr = json::stringify(*value);
    std::cout << "Pushing JSON string: " << jsonStr << "\n";
    lua_pushstring(L, jsonStr.c_str());
    std::cout << "json.encode completed\n";
    return 1;
}