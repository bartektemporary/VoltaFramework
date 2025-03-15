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

int l_input_getPressedKeys(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getPressedKeys: Framework or window is null\n";
        // Return an empty table
        lua_newtable(L);
        return 1;
    }

    // Create a new table to store pressed keys
    lua_newtable(L);
    int index = 1;

    // Iterate through the keyMap (assuming keyMap from Maps.hpp maps string names to GLFW key codes)
    for (const auto& pair : keyMap) {
        int keyCode = pair.second;
        if (glfwGetKey(framework->getWindow(), keyCode) == GLFW_PRESS) {
            // Push the index
            lua_pushnumber(L, index);
            // Push the key name
            lua_pushstring(L, pair.first.c_str());
            // Set the value in the table
            lua_settable(L, -3);
            index++;
        }
    }

    return 1;
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

int l_input_getPressedMouseButtons(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getPressedMouseButtons: Framework or window is null\n";
        // Return an empty table
        lua_newtable(L);
        return 1;
    }

    // Create a new table to store pressed mouse buttons
    lua_newtable(L);
    int index = 1;

    // Define mouse button mappings
    struct ButtonMapping {
        int buttonCode;
        const char* name;
    };

    const ButtonMapping buttons[] = {
        {GLFW_MOUSE_BUTTON_LEFT, "left"},
        {GLFW_MOUSE_BUTTON_RIGHT, "right"},
        {GLFW_MOUSE_BUTTON_MIDDLE, "middle"},
        // Add more buttons if needed (GLFW_MOUSE_BUTTON_4, etc.)
        {0, nullptr} // Sentinel
    };

    // Check each mouse button
    for (int i = 0; buttons[i].name != nullptr; i++) {
        if (glfwGetMouseButton(framework->getWindow(), buttons[i].buttonCode) == GLFW_PRESS) {
            // Push the index
            lua_pushnumber(L, index);
            // Push the button name
            lua_pushstring(L, buttons[i].name);
            // Set the value in the table
            lua_settable(L, -3);
            index++;
        }
    }

    return 1;
}

int l_input_isGamepadConnected(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    int gamepadId = luaL_checkinteger(L, 1);
    bool connected = framework && framework->isGamepadConnected(gamepadId);
    lua_pushboolean(L, connected);
    return 1;
}

int l_input_isGamepadButtonDown(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    int gamepadId = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    bool down = framework && framework->isGamepadButtonDown(gamepadId, button);
    lua_pushboolean(L, down);
    return 1;
}

int l_input_gamepadConnected(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 1)) {
        lua_pushvalue(L, 1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        framework->registerGamepadConnectedCallback(ref);
    } else {
        std::cerr << "Invalid callback function for gamepadConnected\n";
    }
    return 0;
}

int l_input_gamepadDisconnected(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 1)) {
        lua_pushvalue(L, 1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        framework->registerGamepadDisconnectedCallback(ref);
    } else {
        std::cerr << "Invalid callback function for gamepadDisconnected\n";
    }
    return 0;
}

int l_input_getGamepadButtonPressed(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    int gamepadId = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    if (framework && lua_isfunction(L, 3)) {
        lua_pushvalue(L, 3);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        framework->registerGamepadButtonPressedCallback(button, ref);
    } else {
        std::cerr << "Invalid callback function for getGamepadButtonPressed\n";
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

void VoltaFramework::joystickCallback(int jid, int event) {
    VoltaFramework* framework = g_frameworkInstance;
    if (!framework || !framework->L) return;

    if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
        framework->gamepadStates[jid] = true;
        // Trigger connected callbacks
        for (int ref : framework->gamepadConnectedCallbackRefs) {
            lua_rawgeti(framework->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(framework->L, -1)) {
                lua_pushinteger(framework->L, jid);
                if (lua_pcall(framework->L, 1, 0, 0) != LUA_OK) {
                    std::cerr << "Gamepad connected callback error: " << lua_tostring(framework->L, -1) << std::endl;
                    lua_pop(framework->L, 1);
                }
            }
            lua_settop(framework->L, 0);
        }
    } else if (event == GLFW_DISCONNECTED) {
        framework->gamepadStates[jid] = false;
        // Trigger disconnected callbacks
        for (int ref : framework->gamepadDisconnectedCallbackRefs) {
            lua_rawgeti(framework->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(framework->L, -1)) {
                lua_pushinteger(framework->L, jid);
                if (lua_pcall(framework->L, 1, 0, 0) != LUA_OK) {
                    std::cerr << "Gamepad disconnected callback error: " << lua_tostring(framework->L, -1) << std::endl;
                    lua_pop(framework->L, 1);
                }
            }
            lua_settop(framework->L, 0);
        }
    }
}

void VoltaFramework::registerGamepadConnectedCallback(int ref) {
    gamepadConnectedCallbackRefs.push_back(ref);
}

void VoltaFramework::registerGamepadDisconnectedCallback(int ref) {
    gamepadDisconnectedCallbackRefs.push_back(ref);
}

void VoltaFramework::registerGamepadButtonPressedCallback(int button, int ref) {
    gamepadButtonPressedCallbackRefs[button].push_back(ref);
}

bool VoltaFramework::isGamepadConnected(int gamepadId) const {
    auto it = gamepadStates.find(gamepadId);
    return it != gamepadStates.end() && it->second;
}

bool VoltaFramework::isGamepadButtonDown(int gamepadId, int button) const {
    if (!isGamepadConnected(gamepadId)) return false;
    GLFWgamepadstate state;
    if (glfwGetGamepadState(gamepadId, &state)) {
        return state.buttons[button] == GLFW_PRESS;
    }
    return false;
}