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

void VoltaFramework::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    VoltaFramework* framework {g_frameworkInstance};
    if (!framework || action != GLFW_PRESS) return;

    auto it {framework->keyPressCallbackRefs.find(key)};
    if (it != framework->keyPressCallbackRefs.end()) {
        for (int ref : it->second) {
            lua_rawgeti(framework->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(framework->L, -1)) {
                if (lua_pcall(framework->L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Key press callback error for key " << key << ": " << lua_tostring(framework->L, -1) << ", stack top: " << lua_gettop(framework->L) << std::endl;
                    lua_pop(framework->L, 1); // Clean up error message
                }
            } else {
                std::cerr << "Invalid callback for key " << key << ", stack top: " << lua_gettop(framework->L) << std::endl;
            }
            lua_settop(framework->L, 0); // Reset stack to avoid corruption
        }
    }
}

void VoltaFramework::registerKeyPressCallback(const std::string& key, int ref) {
    auto it {keyMap.find(key)};
    if (it != keyMap.end()) {
        keyPressCallbackRefs[it->second].push_back(ref);
    } else {
        std::cerr << "Unknown key: " << key << std::endl;
    }
}

void VoltaFramework::registerMouseButtonPressCallback(const std::string& button, int ref) {
    int buttonCode {GLFW_MOUSE_BUTTON_LEFT};
    if (button == "left") {
        buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    } else if (button == "right") {
        buttonCode = GLFW_MOUSE_BUTTON_RIGHT;
    } else if (button == "middle") {
        buttonCode = GLFW_MOUSE_BUTTON_MIDDLE;
    } else {
        std::cerr << "Unknown mouse button: " << button << std::endl;
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return;
    }
    mouseButtonCallbackRefs[buttonCode].push_back(ref);
}

void VoltaFramework::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    VoltaFramework* framework {g_frameworkInstance};
    if (!framework || action != GLFW_PRESS || !framework->L) return;

    auto it {framework->mouseButtonCallbackRefs.find(button)};
    if (it != framework->mouseButtonCallbackRefs.end()) {
        for (int ref : it->second) {
            lua_rawgeti(framework->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(framework->L, -1)) {
                if (lua_pcall(framework->L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Mouse button callback error for button " << button 
                              << ": " << lua_tostring(framework->L, -1) << std::endl;
                    lua_pop(framework->L, 1);
                }
            } else {
                std::cerr << "Invalid callback reference for button " << button << std::endl;
                lua_pop(framework->L, 1);
            }
        }
        lua_settop(framework->L, 0); // Clear stack after all callbacks
    }
}