#include "VoltaFramework.hpp"
#include "Maps.hpp"

bool VoltaFramework::isKeyDown(const std::string& key) const {
    if (!window) return false;
    std::string k = key;
    for (char& c : k) c = std::tolower(c);
    auto it = keyMap.find(k);
    if (it == keyMap.end()) {
        std::cerr << "isKeyDown: Unknown key '" << k << "'\n";
        return false;
    }
    return glfwGetKey(window, it->second) == GLFW_PRESS;
}

bool VoltaFramework::isMouseButtonDown(const std::string& button) const {
    if (!window) return false;
    std::string btn = button;
    for (char& c : btn) c = std::tolower(c);
    int buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    if (btn == "left") buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    else if (btn == "right") buttonCode = GLFW_MOUSE_BUTTON_RIGHT;
    else if (btn == "middle") buttonCode = GLFW_MOUSE_BUTTON_MIDDLE;
    else {
        std::cerr << "isMouseButtonDown: Unknown button '" << btn << "'\n";
        return false;
    }
    return glfwGetMouseButton(window, buttonCode) == GLFW_PRESS;
}

void VoltaFramework::getMousePosition(double& x, double& y) const {
    if (!window) {
        x = y = 0.0;
        return;
    }
    glfwGetCursorPos(window, &x, &y);
}

std::vector<std::string> VoltaFramework::getPressedKeys() const {
    std::vector<std::string> pressedKeys;
    if (!window) return pressedKeys;
    for (const auto& pair : keyMap) {
        if (glfwGetKey(window, pair.second) == GLFW_PRESS) {
            pressedKeys.push_back(pair.first);
        }
    }
    return pressedKeys;
}

std::vector<std::string> VoltaFramework::getPressedMouseButtons() const {
    std::vector<std::string> pressedButtons;
    if (!window) return pressedButtons;
    const struct { int code; const char* name; } buttons[] = {
        {GLFW_MOUSE_BUTTON_LEFT, "left"},
        {GLFW_MOUSE_BUTTON_RIGHT, "right"},
        {GLFW_MOUSE_BUTTON_MIDDLE, "middle"},
        {0, nullptr}
    };
    for (int i = 0; buttons[i].name; i++) {
        if (glfwGetMouseButton(window, buttons[i].code) == GLFW_PRESS) {
            pressedButtons.push_back(buttons[i].name);
        }
    }
    return pressedButtons;
}

int l_isKeyDown(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushboolean(L, false);
        return 1;
    }
    const char* key = luaL_checkstring(L, 1);
    std::string k = key;
    for (char& c : k) c = std::tolower(c);
    lua_pushboolean(L, framework->isKeyDown(k));
    return 1;
}

int l_input_keyPressed(lua_State* L) {
    const char* key = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 2)) {
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        framework->registerKeyPressCallback(key, ref);
    } else {
        std::cerr << "Invalid callback function for key: " << key << std::endl;
    }
    return 0;
}

int l_input_getPressedKeys(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_newtable(L);
        return 1;
    }

    auto pressedKeys = framework->getPressedKeys();
    lua_newtable(L);
    for (size_t i = 0; i < pressedKeys.size(); ++i) {
        lua_pushnumber(L, i + 1); // Lua tables are 1-indexed
        lua_pushstring(L, pressedKeys[i].c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int l_input_isMouseButtonDown(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushboolean(L, false);
        return 1;
    }
    const char* button = luaL_checkstring(L, 1);
    std::string btn = button;
    for (char& c : btn) c = std::tolower(c);
    lua_pushboolean(L, framework->isMouseButtonDown(btn));
    return 1;
}

int l_input_getMousePosition(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    }
    double x, y;
    framework->getMousePosition(x, y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

int l_input_mouseButtonPressed(lua_State* L) {
    const char* button = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (framework && lua_isfunction(L, 2)) {
        std::string btn = button;
        for (char& c : btn) c = std::tolower(c);
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        framework->registerMouseButtonPressCallback(btn, ref);
    } else {
        std::cerr << "Invalid callback function for mouse button: " << button << std::endl;
    }
    return 0;
}

int l_input_getPressedMouseButtons(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_newtable(L);
        return 1;
    }

    auto pressedButtons = framework->getPressedMouseButtons();
    lua_newtable(L);
    for (size_t i = 0; i < pressedButtons.size(); ++i) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, pressedButtons[i].c_str());
        lua_settable(L, -3);
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

void VoltaFramework::registerGamepadConnectedCallback(int ref) {
    gamepadConnectedCallbackRefs.push_back(ref);
}

void VoltaFramework::registerGamepadDisconnectedCallback(int ref) {
    gamepadDisconnectedCallbackRefs.push_back(ref);
}

void VoltaFramework::registerGamepadButtonPressedCallback(int button, int ref) {
    gamepadButtonPressedCallbackRefs[button].push_back(ref);
}

void VoltaFramework::registerCppKeyPressCallback(const std::string& key, std::function<void()> callback) {
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        cppKeyPressCallbacks[it->second].push_back(std::move(callback));
    } else {
        std::cerr << "registerCppKeyPressCallback: Unknown key '" << key << "'\n";
    }
}

void VoltaFramework::registerCppMouseButtonPressCallback(const std::string& button, std::function<void()> callback) {
    int buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    if (button == "left") buttonCode = GLFW_MOUSE_BUTTON_LEFT;
    else if (button == "right") buttonCode = GLFW_MOUSE_BUTTON_RIGHT;
    else if (button == "middle") buttonCode = GLFW_MOUSE_BUTTON_MIDDLE;
    else {
        std::cerr << "registerCppMouseButtonPressCallback: Unknown button '" << button << "'\n";
        return;
    }
    cppMouseButtonPressCallbacks[buttonCode].push_back(std::move(callback));
}

void VoltaFramework::registerCppGamepadConnectedCallback(std::function<void(int)> callback) {
    cppGamepadConnectedCallbacks.push_back(std::move(callback));
}

void VoltaFramework::registerCppGamepadDisconnectedCallback(std::function<void(int)> callback) {
    cppGamepadDisconnectedCallbacks.push_back(std::move(callback));
}

void VoltaFramework::registerCppGamepadButtonPressedCallback(int button, std::function<void(int)> callback) {
    cppGamepadButtonPressedCallbacks[button].push_back(std::move(callback));
}

void VoltaFramework::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!g_frameworkInstance || action != GLFW_PRESS) return;

    // Lua callbacks
    auto luaIt = g_frameworkInstance->keyPressCallbackRefs.find(key);
    if (luaIt != g_frameworkInstance->keyPressCallbackRefs.end()) {
        for (int ref : luaIt->second) {
            lua_rawgeti(g_frameworkInstance->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(g_frameworkInstance->L, -1)) {
                if (lua_pcall(g_frameworkInstance->L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Key press callback error for key " << key << ": " << lua_tostring(g_frameworkInstance->L, -1) << "\n";
                    lua_pop(g_frameworkInstance->L, 1);
                }
            } else {
                lua_pop(g_frameworkInstance->L, 1);
            }
        }
        lua_settop(g_frameworkInstance->L, 0);
    }

    // C++ callbacks
    auto cppIt = g_frameworkInstance->cppKeyPressCallbacks.find(key);
    if (cppIt != g_frameworkInstance->cppKeyPressCallbacks.end()) {
        for (auto& callback : cppIt->second) {
            if (callback) callback();
        }
    }
}

void VoltaFramework::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (!g_frameworkInstance || action != GLFW_PRESS || !g_frameworkInstance->L) return;

    // Lua callbacks
    auto luaIt = g_frameworkInstance->mouseButtonCallbackRefs.find(button);
    if (luaIt != g_frameworkInstance->mouseButtonCallbackRefs.end()) {
        for (int ref : luaIt->second) {
            lua_rawgeti(g_frameworkInstance->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(g_frameworkInstance->L, -1)) {
                if (lua_pcall(g_frameworkInstance->L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Mouse button callback error for button " << button << ": " << lua_tostring(g_frameworkInstance->L, -1) << "\n";
                    lua_pop(g_frameworkInstance->L, 1);
                }
            } else {
                lua_pop(g_frameworkInstance->L, 1);
            }
        }
        lua_settop(g_frameworkInstance->L, 0);
    }

    // C++ callbacks
    auto cppIt = g_frameworkInstance->cppMouseButtonPressCallbacks.find(button);
    if (cppIt != g_frameworkInstance->cppMouseButtonPressCallbacks.end()) {
        for (auto& callback : cppIt->second) {
            if (callback) callback();
        }
    }
}

void VoltaFramework::joystickCallback(int jid, int event) {
    if (!g_frameworkInstance || !g_frameworkInstance->L) return;

    if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
        g_frameworkInstance->gamepadStates[jid] = true;

        // Lua connected callbacks
        for (int ref : g_frameworkInstance->gamepadConnectedCallbackRefs) {
            lua_rawgeti(g_frameworkInstance->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(g_frameworkInstance->L, -1)) {
                lua_pushinteger(g_frameworkInstance->L, jid);
                if (lua_pcall(g_frameworkInstance->L, 1, 0, 0) != LUA_OK) {
                    std::cerr << "Gamepad connected callback error: " << lua_tostring(g_frameworkInstance->L, -1) << "\n";
                    lua_pop(g_frameworkInstance->L, 1);
                }
            }
            lua_settop(g_frameworkInstance->L, 0);
        }

        // C++ connected callbacks
        for (auto& callback : g_frameworkInstance->cppGamepadConnectedCallbacks) {
            if (callback) callback(jid);
        }
    } else if (event == GLFW_DISCONNECTED) {
        g_frameworkInstance->gamepadStates[jid] = false;

        // Lua disconnected callbacks
        for (int ref : g_frameworkInstance->gamepadDisconnectedCallbackRefs) {
            lua_rawgeti(g_frameworkInstance->L, LUA_REGISTRYINDEX, ref);
            if (lua_isfunction(g_frameworkInstance->L, -1)) {
                lua_pushinteger(g_frameworkInstance->L, jid);
                if (lua_pcall(g_frameworkInstance->L, 1, 0, 0) != LUA_OK) {
                    std::cerr << "Gamepad disconnected callback error: " << lua_tostring(g_frameworkInstance->L, -1) << "\n";
                    lua_pop(g_frameworkInstance->L, 1);
                }
            }
            lua_settop(g_frameworkInstance->L, 0);
        }

        // C++ disconnected callbacks
        for (auto& callback : g_frameworkInstance->cppGamepadDisconnectedCallbacks) {
            if (callback) callback(jid);
        }
    }
}