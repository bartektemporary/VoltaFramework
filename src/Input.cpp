#include "VoltaFramework.hpp"
#include "Maps.hpp"

int l_isKeyDown(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "isKeyDown: Framework or window is null\n";
        lua_pushboolean(L, false);
        return 1;
    }
    const char* key {luaL_checkstring(L, 1)};
    std::string k {key};
    
    for (char& c : k) {
        c = std::tolower(c);
    }

    int keyCode {GLFW_KEY_UNKNOWN};
    auto it {keyMap.find(k)};
    if (it != keyMap.end()) {
        keyCode = it->second;
    } else {
        std::cerr << "isKeyDown: Unknown key '" << k << "'\n";
    }

    bool isDown {(keyCode != GLFW_KEY_UNKNOWN) && 
                 (glfwGetKey(framework->getWindow(), keyCode) == GLFW_PRESS)};
    lua_pushboolean(L, isDown);
    return 1;
}

int l_input_keyPressed(lua_State* L) {
    const char* key {luaL_checkstring(L, 1)};
    VoltaFramework* framework {getFramework(L)};
    if (framework && lua_isfunction(L, 2)) {
        lua_pushvalue(L, 2);
        int ref {luaL_ref(L, LUA_REGISTRYINDEX)};
        framework->registerKeyPressCallback(key, ref);
    } else {
        std::cerr << "Invalid callback function for key: " << key << std::endl;
    }
    return 0;
}

int l_input_isMouseButtonDown(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "isMouseButtonDown: Framework or window is null\n";
        lua_pushboolean(L, false);
        return 1;
    }

    const char* button {luaL_checkstring(L, 1)};
    std::string btn {button};
    for (char& c : btn) {
        c = std::tolower(c);
    }

    int buttonCode {GLFW_MOUSE_BUTTON_LEFT};
    if (btn == "left") {
        buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    } else if (btn == "right") {
        buttonCode = GLFW_MOUSE_BUTTON_RIGHT;
    } else if (btn == "middle") {
        buttonCode = GLFW_MOUSE_BUTTON_MIDDLE;
    } else {
        std::cerr << "isMouseButtonDown: Unknown mouse button '" << btn << "'\n";
    }

    bool isDown {glfwGetMouseButton(framework->getWindow(), buttonCode) == GLFW_PRESS};
    lua_pushboolean(L, isDown);
    return 1;
}

int l_input_getMousePosition(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "getMousePosition: Framework or window is null\n";
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    }

    double xpos {}, ypos {};
    glfwGetCursorPos(framework->getWindow(), &xpos, &ypos);
    
    lua_pushnumber(L, xpos);
    lua_pushnumber(L, ypos);
    return 2;
}

int l_input_mouseButtonPressed(lua_State* L) {
    const char* button {luaL_checkstring(L, 1)};
    VoltaFramework* framework {getFramework(L)};
    if (framework && lua_isfunction(L, 2)) {
        std::string btn {button};
        for (char& c : btn) {
            c = std::tolower(c);
        }
        
        lua_pushvalue(L, 2);
        int ref {luaL_ref(L, LUA_REGISTRYINDEX)};
        framework->registerMouseButtonPressCallback(btn, ref);
    } else {
        std::cerr << "Invalid callback function for mouse button: " << button << std::endl;
    }
    return 0;
}