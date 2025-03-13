#include "VoltaFramework.hpp"

void VoltaFramework::registerCustomEventCallback(const std::string& eventName, int ref) {
    customEventCallbackRefs[eventName].push_back(ref);
}

void VoltaFramework::triggerCustomEvent(const std::string& eventName, lua_State* L, int nargs) {
    auto it = customEventCallbackRefs.find(eventName);
    if (it == customEventCallbackRefs.end()) return; // No callbacks registered for this event

    for (int ref : it->second) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (lua_isfunction(L, -1)) {
            // Push all arguments passed to triggerCustomEvent
            for (int i = 1; i <= nargs; i++) {
                lua_pushvalue(L, -nargs - 1); // Copy argument from stack
            }
            if (lua_pcall(L, nargs, 0, 0) != LUA_OK) {
                std::cerr << "Custom event callback error for event '" << eventName 
                          << "': " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        } else {
            std::cerr << "Invalid callback for event '" << eventName << "'\n";
            lua_pop(L, 1);
        }
    }
    lua_pop(L, nargs); // Remove arguments from stack
}

int l_onCustomEvent(lua_State* L) {
    const char* eventName = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 2)) {
        lua_pushvalue(L, 2); // Copy the callback function
        int ref = luaL_ref(L, LUA_REGISTRYINDEX); // Store reference in registry
        framework->registerCustomEventCallback(eventName, ref);
    } else {
        std::cerr << "Invalid callback function for custom event: " << eventName << std::endl;
    }
    return 0;
}

int l_triggerCustomEvent(lua_State* L) {
    const char* eventName = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework) {
        int nargs = lua_gettop(L) - 1; // Number of arguments after eventName
        framework->triggerCustomEvent(eventName, L, nargs);
    } else {
        std::cerr << "Cannot trigger custom event: Framework is null\n";
    }
    return 0;
}