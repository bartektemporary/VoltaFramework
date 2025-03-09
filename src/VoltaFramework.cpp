#include "VoltaFramework.hpp"
#include <cstring>
#include "Maps.hpp"

// Global instance pointer
VoltaFramework* g_frameworkInstance {nullptr};

VoltaFramework::VoltaFramework() : L{luaL_newstate()}, width{800}, height{600}, x{100}, y{100} {
    luaL_openlibs(L);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Lua OpenGL Game", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        exit(1);
    }

    glViewport(0, 0, width, height);
    glfwSetWindowPos(window, x, y); // Set initial position

    // Set global instance and register size callback
    g_frameworkInstance = this;
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetWindowPosCallback(window, windowPosCallback);
    glfwSetWindowMaximizeCallback(window, windowMaximizeCallback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "VoltaFrameworkInstance");

    FreeImage_Initialise();

    filterMode = GL_LINEAR;
    globalVolume = 1.0f;

    // Initialize miniaudio engine
    ma_result result {ma_engine_init(NULL, &engine)};
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio engine\n";
        exit(1);
    }

    registerLuaAPI();
    std::cout << "Framework initialized successfully\n";
}

VoltaFramework::~VoltaFramework() {
    g_frameworkInstance = nullptr;
    for (auto& pair : audioCache) {
        ma_sound_uninit(&pair.second);
    }
    audioCache.clear();
    for (auto& pair : keyPressCallbackRefs) {
        for (int ref : pair.second) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
    }
    keyPressCallbackRefs.clear();
    for (auto& pair : mouseButtonCallbackRefs) {
        for (int ref : pair.second) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
    }
    mouseButtonCallbackRefs.clear();
    ma_engine_uninit(&engine);
    glfwDestroyWindow(window);
    glfwTerminate();
    lua_close(L);
}

void VoltaFramework::run() {
    loadLuaScript("scripts/main.lua");

    lua_getglobal(L, "init");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "Init Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        } else {
        }
    } else {
        lua_pop(L, 1);
    }

    double lastTime {glfwGetTime()};
    while (!glfwWindowShouldClose(window)) {
        double currentTime {glfwGetTime()};
        float dt {static_cast<float>(currentTime - lastTime)};
        lastTime = currentTime;

        glfwPollEvents();
        update(dt);
        glfwSwapBuffers(window);
    }
}

void VoltaFramework::loadLuaScript(const std::string& filename) {
    if (luaL_dofile(L, filename.c_str()) != LUA_OK ) {
        std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}

void VoltaFramework::update(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);

    lua_getglobal(L, "update");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            std::cerr << "Update Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
}

void VoltaFramework::windowSizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
    if (g_frameworkInstance && g_frameworkInstance->getWindow() == window) {
        g_frameworkInstance->width = newWidth;
        g_frameworkInstance->height = newHeight;
        glViewport(0, 0, newWidth, newHeight);
    }
}

void VoltaFramework::windowPosCallback(GLFWwindow* window, int xpos, int ypos) {
    if (g_frameworkInstance && g_frameworkInstance->getWindow() == window) {
        g_frameworkInstance->x = xpos;
        g_frameworkInstance->y = ypos;
    }
}

void VoltaFramework::windowMaximizeCallback(GLFWwindow* window, int maximized) {
    if (g_frameworkInstance && g_frameworkInstance->getWindow() == window) {
        if (maximized) {
            g_frameworkInstance->state = 2; // Maximized
        } else if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            g_frameworkInstance->state = 1; // Minimized (check if iconified)
        } else {
            g_frameworkInstance->state = 0; // Normal
        }
    }
}

GLuint VoltaFramework::loadTexture(const std::string& filename) {
    if (textureCache.find(filename) != textureCache.end()) {
        return textureCache[filename];
    }

    FREE_IMAGE_FORMAT format {FreeImage_GetFileType(filename.c_str(), 0)};
    if (format == FIF_UNKNOWN) {
        format = FreeImage_GetFIFFromFilename(filename.c_str());
    }
    if (format == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(format)) {
        std::cerr << "Unsupported image format: " << filename << std::endl;
        return 0;
    }

    FIBITMAP* bitmap {FreeImage_Load(format, filename.c_str())};
    if (!bitmap) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return 0;
    }

    FIBITMAP* bitmap32 {FreeImage_ConvertTo32Bits(bitmap)};
    FreeImage_Unload(bitmap);
    if (!bitmap32) {
        std::cerr << "Failed to convert image to 32-bit: " << filename << std::endl;
        return 0;
    }

    unsigned int width {FreeImage_GetWidth(bitmap32)};
    unsigned int height {FreeImage_GetHeight(bitmap32)};
    void* pixels {FreeImage_GetBits(bitmap32)};

    GLuint textureID {};
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

    // Apply current filter mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    FreeImage_Unload(bitmap32);

    textureCache[filename] = textureID;
    return textureID;
}

ma_sound* VoltaFramework::loadAudio(const std::string& filename) {
    if (audioCache.find(filename) != audioCache.end()) {
        return &audioCache[filename];
    }

    std::string fullPath {std::string("assets/") + filename};
    ma_sound* sound {&audioCache[filename]};
    ma_result result {ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, sound)};
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio file: " << fullPath << " (Error: " << result << ")" << std::endl;
        audioCache.erase(filename);
        return nullptr;
    }
    // Apply initial global volume
    ma_sound_set_volume(sound, ma_sound_get_volume(sound) * globalVolume);
    return sound;
}

void VoltaFramework::setGlobalVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    // If current volume is 0, reset individual sound volumes instead of scaling
    if (globalVolume == 0.0f) {
        for (auto& pair : audioCache) {
            ma_sound* sound {&pair.second};
            ma_sound_set_volume(sound, volume);
        }
    } else {
        float scaleFactor {volume / globalVolume};
        for (auto& pair : audioCache) {
            ma_sound* sound {&pair.second};
            float currentVolume {ma_sound_get_volume(sound)};
            ma_sound_set_volume(sound, currentVolume * scaleFactor);
        }
    }
    
    globalVolume = volume;
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

VoltaFramework* getFramework(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "VoltaFrameworkInstance");
    VoltaFramework* framework {static_cast<VoltaFramework*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);
    if (!framework) {
        std::cerr << "Error: Framework instance is null!\n";
    }
    return framework;
}