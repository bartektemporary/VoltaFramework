#include "VoltaFramework.hpp"

void VoltaFramework::newEvent(const std::string& eventName) {
    if (eventCallbacks.find(eventName) == eventCallbacks.end()) {
        eventCallbacks[eventName] = std::vector<EventCallback>();
    }
}

// Register a Lua callback
void VoltaFramework::onEvent(const std::string& eventName, int luaRef) {
    if (eventCallbacks.find(eventName) != eventCallbacks.end()) {
        eventCallbacks[eventName].emplace_back(luaRef);
    } else {
        luaL_unref(L, LUA_REGISTRYINDEX, luaRef); // Clean up unused ref
    }
}

// Register a C++ callback
void VoltaFramework::onEvent(const std::string& eventName, std::function<void(lua_State*, int)> cppCallback) {
    if (eventCallbacks.find(eventName) != eventCallbacks.end()) {
        eventCallbacks[eventName].emplace_back(cppCallback);
    }
}

// Trigger an event
void VoltaFramework::triggerEvent(const std::string& eventName, lua_State* L, int nargs) {
    auto it = eventCallbacks.find(eventName);
    if (it == eventCallbacks.end()) return; // Event not defined or no callbacks

    for (const auto& cb : it->second) {
        if (cb.isLua) {
            // Lua callback
            int ref = std::get<int>(cb.callback);
            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(L, -1)) {
                // Push arguments
                for (int i = 1; i <= nargs; i++) {
                    lua_pushvalue(L, -nargs - 1);
                }
                if (lua_pcall(L, nargs, 0, 0) != LUA_OK) {
                    std::cerr << "Event callback error for '" << eventName 
                              << "': " << lua_tostring(L, -1) << std::endl;
                    lua_pop(L, 1);
                }
            } else {
                std::cerr << "Invalid Lua callback for event '" << eventName << "'\n";
                lua_pop(L, 1);
            }
        } else {
            // C++ callback
            auto cppFunc = std::get<std::function<void(lua_State*, int)>>(cb.callback);
            cppFunc(L, nargs);
        }
    }
    if (nargs > 0) lua_pop(L, nargs);
}

// Check if an event is declared
bool VoltaFramework::hasEvent(const std::string& eventName) const {
    return eventCallbacks.find(eventName) != eventCallbacks.end();
}

int l_event_new(lua_State* L) {
    const char* eventName = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework) {
        framework->newEvent(eventName);
    }
    return 0;
}

int l_event_on(lua_State* L) {
    const char* eventName = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 2)) {
        lua_pushvalue(L, 2); // Copy the callback function
        int ref = luaL_ref(L, LUA_REGISTRYINDEX); // Store in registry
        framework->onEvent(eventName, ref);
    } else {
        std::cerr << "Invalid callback function for event: " << eventName << std::endl;
    }
    return 0;
}

int l_event_trigger(lua_State* L) {
    const char* eventName = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework) {
        int nargs = lua_gettop(L) - 1; // Number of arguments after eventName
        framework->triggerEvent(eventName, L, nargs);
    } else {
        std::cerr << "Cannot trigger event: Framework is null\n";
    }
    return 0;
}