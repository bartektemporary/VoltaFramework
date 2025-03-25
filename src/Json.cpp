#include "VoltaFramework.hpp"

std::unique_ptr<json::Value> VoltaFramework::decodeJson(const std::string& jsonStr) const {
    try {
        return json::parse(jsonStr);
    } catch (const json::JsonException& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return nullptr;
    }
}

std::string VoltaFramework::encodeJson(const json::Value& value) const {
    return json::stringify(value);
}

std::unique_ptr<json::Value> VoltaFramework::createJsonNull() const {
    return std::make_unique<json::Null>();
}

std::unique_ptr<json::Value> VoltaFramework::createJsonBoolean(bool value) const {
    return std::make_unique<json::Boolean>(value);
}

std::unique_ptr<json::Value> VoltaFramework::createJsonNumber(double value) const {
    return std::make_unique<json::Number>(value);
}

std::unique_ptr<json::Value> VoltaFramework::createJsonString(const std::string& value) const {
    return std::make_unique<json::String>(value);
}

std::unique_ptr<json::Value> VoltaFramework::createJsonArray() const {
    return std::make_unique<json::Array>();
}

std::unique_ptr<json::Value> VoltaFramework::createJsonObject() const {
    return std::make_unique<json::Object>();
}

json::Value* VoltaFramework::luaToJson(lua_State* L, int index) {
    int top = lua_gettop(L); // Save initial stack size
    switch (lua_type(L, index)) {
        case LUA_TNIL:
            return new json::Null();
        case LUA_TBOOLEAN:
            return new json::Boolean(lua_toboolean(L, index));
        case LUA_TNUMBER:
            return new json::Number(lua_tonumber(L, index));
        case LUA_TSTRING:
            return new json::String(lua_tostring(L, index));
        case LUA_TTABLE: {
            // Convert index to absolute to avoid issues with relative indexing
            int absIndex = lua_absindex(L, index);
            json::Array* arr = new json::Array();
            json::Object* obj = new json::Object();
            bool isArray = true;
            size_t arrayIndex = 1;

            lua_pushnil(L); // Start iteration
            while (lua_next(L, absIndex)) { // Use absolute index
                if (lua_type(L, -2) == LUA_TNUMBER && lua_tonumber(L, -2) == arrayIndex) {
                    arr->add(std::unique_ptr<json::Value>(luaToJson(L, -1)));
                    arrayIndex++;
                } else {
                    isArray = false;
                    std::string key = lua_tostring(L, -2) ? lua_tostring(L, -2) : std::to_string(lua_tonumber(L, -2));
                    obj->set(key, std::unique_ptr<json::Value>(luaToJson(L, -1)));
                }
                lua_pop(L, 1); // Pop value, keep key for next iteration
            }

            json::Value* result = isArray ? static_cast<json::Value*>(arr) : static_cast<json::Value*>(obj);
            if (isArray) {
                delete obj;
            } else {
                delete arr;
            }

            int newTop = lua_gettop(L);
            if (newTop != top) {
                std::cerr << "Stack imbalance in luaToJson: expected " << top << ", got " << newTop << "\n";
                lua_settop(L, top); // Restore original stack
            }
            return result;
        }
        default:
            luaL_error(L, "Unsupported Lua type for JSON conversion");
            return new json::Null(); // Fallback
    }
}

// Helper to convert json::Value to Lua value
void VoltaFramework::jsonToLua(lua_State* L, const json::Value& value) {
    switch (value.type()) {
        case json::Type::Null:
            lua_pushnil(L);
            break;
        case json::Type::Boolean:
            lua_pushboolean(L, value.asBoolean());
            break;
        case json::Type::Number:
            lua_pushnumber(L, value.asNumber());
            break;
        case json::Type::String:
            lua_pushstring(L, value.asString().c_str());
            break;
        case json::Type::Array: {
            const json::Array& arr = value.asArray();
            lua_newtable(L);
            for (size_t i = 0; i < arr.size(); ++i) {
                lua_pushnumber(L, i + 1); // Lua arrays are 1-based
                jsonToLua(L, arr.at(i));
                lua_settable(L, -3);
            }
            break;
        }
        case json::Type::Object: {
            const json::Object& obj = value.asObject();
            lua_newtable(L);
            for (const auto& key : obj.keys()) {
                lua_pushstring(L, key.c_str());
                jsonToLua(L, obj.get(key));
                lua_settable(L, -3);
            }
            break;
        }
    }
}

int l_json_decode(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        luaL_error(L, "Expected string argument for json.decode");
        return 0;
    }

    const char* jsonStr = lua_tostring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }

    std::unique_ptr<json::Value> value = framework->decodeJson(jsonStr);
    if (!value) {
        luaL_error(L, "Failed to parse JSON string");
        return 0;
    }

    framework->jsonToLua(L, *value);
    return 1;
}

int l_json_encode(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }

    std::unique_ptr<json::Value> value(framework->luaToJson(L, 1));
    std::string jsonStr = framework->encodeJson(*value);
    lua_pushstring(L, jsonStr.c_str());
    return 1;
}