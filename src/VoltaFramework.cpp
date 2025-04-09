#include "VoltaFramework.hpp"
#include <fstream>
#include <cstring>
#include "Maps.hpp"

void VoltaFramework::updateParticles(float dt) {
    for (auto& emitter : particleEmitters) {
        emitter.update(dt);
    }
}

VoltaFramework* g_frameworkInstance {nullptr};

class DefaultGame : public GameBase {
    public:
        void init(VoltaFramework* framework) override {
            std::cout << "Default C++ Game Initialized\n";
        }
        void update(VoltaFramework* framework, float dt) override {
            // Empty default update
        }
};

VoltaFramework::VoltaFramework() : 
    L{nullptr}, 
    window{nullptr}, 
    width{800}, 
    height{600}, 
    x{100}, 
    y{100}, 
    startTime{0.0},
    usingCustomShader(false),
    cppGame{nullptr},
    ftLibrary{nullptr},
    ftFace{nullptr},
    textVAO{0},
    textVBO{0},
    textureVAO{0},
    textureVBO{0},
    particleVAO{0},
    particleVBO{0},
    globalVolume(1.0f),
    eventCallbacks()
{
    L = luaL_newstate();
    if (!L) {
        std::cerr << "Failed to create Lua state\n";
        exit(1);
    }
    luaL_openlibs(L);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        lua_close(L);
        exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lua OpenGL Game", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        lua_close(L);
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        lua_close(L);
        glfwTerminate();
        exit(1);
    }

    initOpenGL();
    glViewport(0, 0, width, height);
    glfwSetWindowPos(window, x, y);

    g_frameworkInstance = this;
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetWindowPosCallback(window, windowPosCallback);
    glfwSetWindowMaximizeCallback(window, windowMaximizeCallback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetJoystickCallback(joystickCallback);

    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++) {
        if (glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid)) {
            gamepadStates[jid] = true;
        }
    }

    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "VoltaFrameworkInstance");

    FreeImage_Initialise();
    filterMode = GL_LINEAR;
    globalVolume = 1.0f;

    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio engine: " << result << "\n";
        FreeImage_DeInitialise();
        glfwDestroyWindow(window);
        lua_close(L);
        glfwTerminate();
        exit(1);
    }

    currentColor = Color(1, 1, 1);

    registerLuaAPI();

    if (FT_Init_FreeType(&ftLibrary) != 0) {
        std::cerr << "Failed to initialize FreeType\n";
    } else if (fs::exists("assets/Minecraft.ttf")) {
        loadFont("Minecraft.ttf", 24);
        setFont("Minecraft.ttf");
    } else {
        std::cerr << "Warning: Minecraft.ttf not found in assets/, text rendering disabled\n";
    }

    startTime = glfwGetTime();
    std::cout << "Framework initialized successfully\n";
}

VoltaFramework::~VoltaFramework() {
    lua_close(L);
    if (cppGame) {
        delete cppGame;
        cppGame = nullptr;
    }
    g_frameworkInstance = nullptr;
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
    for (auto& pair : textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    for (int ref : gamepadConnectedCallbackRefs) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
    for (int ref : gamepadDisconnectedCallbackRefs) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
    for (auto& pair : gamepadButtonPressedCallbackRefs) {
        for (int ref : pair.second) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
    }

    gamepadConnectedCallbackRefs.clear();
    gamepadDisconnectedCallbackRefs.clear();
    gamepadButtonPressedCallbackRefs.clear();
    gamepadStates.clear();

    cppKeyPressCallbacks.clear();
    cppMouseButtonPressCallbacks.clear();
    cppGamepadConnectedCallbacks.clear();
    cppGamepadDisconnectedCallbacks.clear();
    cppGamepadButtonPressedCallbacks.clear();

    textureCache.clear();
    FreeImage_DeInitialise();
    cleanupOpenGL();
    for (auto& pair : eventCallbacks) {
        for (auto& cb : pair.second) {
            if (cb.isLua) {
                luaL_unref(L, LUA_REGISTRYINDEX, std::get<int>(cb.callback));
            }
        }
    }
    eventCallbacks.clear();
    audioCache.clear();
    ma_engine_uninit(&engine);
    for (auto& pair : databaseCache) {
        sqlite3_close(pair.second);
    }
    databaseCache.clear();
    for (auto& pair : fontCache) {
        FT_Done_Face(pair.second);
    }
    fontCache.clear();
    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
    }
    if (window) {
        glfwDestroyWindow(window);
    }

    currentCamera = nullptr;
    currentCamera3D = nullptr;

    glfwTerminate();
}

std::string VoltaFramework::loadFile(const std::string& filename, bool asText) {
    fs::path fullPath;

    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        fullPath = fs::path(filename);
    } else {
        fullPath = fs::path("assets") / filename;
    }

    fullPath = fullPath.lexically_normal();

    if (!fs::exists(fullPath)) {
        std::cerr << "File not found: " << fullPath.string() << std::endl;
        return "";
    }

    if (asText) {
        std::ifstream fileStream(fullPath, std::ios::binary);
        if (!fileStream.is_open()) {
            std::cerr << "Failed to open file: " << fullPath.string() << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        fileStream.close();
        return buffer.str();
    }

    return fullPath.string();
}

void VoltaFramework::run() {
    bool hasLua = fs::exists("scripts/main.lua");
    bool hasCpp = fs::exists("src/main.cpp");

    if (hasLua) {
        loadLuaScript("scripts/main.lua");
    }

    if (hasCpp && !cppGame) {
        std::cout << "C++ game detected, assuming set by main()\n";
    }

    if (hasLua) {
        lua_getglobal(L, "volta");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "init");
            if (lua_isfunction(L, -1)) {
                if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Init Error (Lua): " << lua_tostring(L, -1) << std::endl;
                    lua_pop(L, 1);
                }
            } else {
                lua_pop(L, 1);
            }
        } else {
            std::cerr << "Error: 'volta' table not found in Lua environment\n";
        }
        lua_pop(L, 1);
    }

    if (cppGame) {
        cppGame->init(this);
    }

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();
        update(dt);
        glfwSwapBuffers(window);
    }
}

void VoltaFramework::loadLuaScript(const std::string& filename) {
    std::string fullPath = loadFile(filename);
    if (fullPath.empty()) {
        return;
    }
    if (luaL_dofile(L, fullPath.c_str()) != LUA_OK) {
        std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}

void VoltaFramework::update(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentCamera3D) {
        setCamera3D(currentCamera3D);
        cachedViewBounds = Rect(-FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX);
    } else if (currentCamera) {
        setCamera2D(currentCamera);
        Vector2 cameraPos = currentCamera->getPosition();
        float zoom = currentCamera->getZoom();
        float halfWidth = (width / 2.0f) / zoom;
        float halfHeight = (height / 2.0f) / zoom;

        cachedViewBounds = Rect(
            cameraPos.x - halfWidth,
            cameraPos.x + halfWidth,
            cameraPos.y - halfHeight,
            cameraPos.y + halfHeight
        );

        float rotation = currentCamera->getRotation();
        if (rotation != 0.0f) {
            float rad = rotation * M_PI / 180.0f;
            float cosR = fabs(cosf(rad));
            float sinR = fabs(sinf(rad));
            float rotatedWidth = halfWidth * cosR + halfHeight * sinR;
            float rotatedHeight = halfWidth * sinR + halfHeight * cosR;
            cachedViewBounds = Rect(
                cameraPos.x - rotatedWidth,
                cameraPos.x + rotatedWidth,
                cameraPos.y - rotatedHeight,
                cameraPos.y + rotatedHeight
            );
        }
    } else {
        setCamera2D(nullptr);
        cachedViewBounds = Rect(-FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX);
    }

    // Poll gamepad button presses
    for (auto& state : gamepadStates) {
        int jid = state.first;
        if (!state.second) continue;
        GLFWgamepadstate gpState;
        if (glfwGetGamepadState(jid, &gpState)) {
            for (auto& btnPair : gamepadButtonPressedCallbackRefs) {
                int button = btnPair.first;
                static std::unordered_map<int, bool> lastButtonStates;
                bool isPressed = gpState.buttons[button] == GLFW_PRESS;
                if (isPressed && !lastButtonStates[button]) {
                    for (int ref : btnPair.second) {
                        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
                        if (lua_isfunction(L, -1)) {
                            lua_pushinteger(L, jid);
                            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                                std::cerr << "Gamepad button callback error: " << lua_tostring(L, -1) << "\n";
                                lua_pop(L, 1);
                            }
                        }
                        lua_settop(L, 0);
                    }
                    auto cppIt = cppGamepadButtonPressedCallbacks.find(button);
                    if (cppIt != cppGamepadButtonPressedCallbacks.end()) {
                        for (auto& callback : cppIt->second) {
                            if (callback) callback(jid);
                        }
                    }
                }
                lastButtonStates[button] = isPressed;
            }
        }
    }

    currentColor = Color(1, 1, 1);
    currentShaderName = "";
    usingCustomShader = false;

    lua_getglobal(L, "volta");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "update");
        if (lua_isfunction(L, -1)) {
            lua_pushnumber(L, dt);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                std::cerr << "Update Error (Lua): " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1);
        }
    } else if (fs::exists("scripts/main.lua")) {
        std::cerr << "Error: 'volta' table not found in Lua environment\n";
    }
    lua_pop(L, 1);

    if (cppGame) {
        cppGame->update(this, dt);
    }

    updateParticles(dt);
}

bool VoltaFramework::loadCppGame() {
    if (fs::exists("src/main.cpp")) {
        cppGame = new DefaultGame();
        return true;
    }
    return false;
}

double VoltaFramework::getRunningTime() const {
    double currentTime = glfwGetTime();
    return currentTime - startTime;
}

int l_getRunningTime(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushnumber(L, 0.0);
        return 1;
    }
    lua_pushnumber(L, framework->getRunningTime());
    return 1;
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