#include "VoltaFramework.hpp"
#include <cstring>
#include "Maps.hpp"

VoltaFramework* g_frameworkInstance {nullptr};

VoltaFramework::VoltaFramework() : 
    L{luaL_newstate()}, 
    width{800}, 
    height{600}, 
    x{100}, 
    y{100}, 
    startTime{glfwGetTime()},
    usingCustomShader(false) {
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

    ma_result result {ma_engine_init(NULL, &engine)};
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio engine\n";
        exit(1);
    }

    registerLuaAPI();
    std::cout << "Framework initialized successfully\n";
}

VoltaFramework::~VoltaFramework() {
    lua_close(L);
    g_frameworkInstance = nullptr;
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
    textureCache.clear();
    FreeImage_DeInitialise();
    cleanupOpenGL();
    for (auto& pair : customEventCallbackRefs) {
        for (int ref : pair.second) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
    }
    customEventCallbackRefs.clear();
    for (auto& pair : audioCache) {
        ma_sound_uninit(&pair.second);
    }
    for (auto& pair : databaseCache) {
        sqlite3_close(pair.second);
    }
    databaseCache.clear();
    ma_engine_uninit(&engine);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VoltaFramework::run() {
    loadLuaScript("scripts/main.lua");

    lua_getglobal(L, "init");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "Init Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
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
    if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
        std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}

void VoltaFramework::update(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentColor[0] = 1.0f;
    currentColor[1] = 1.0f;
    currentColor[2] = 1.0f;
    currentShaderName = ""; // Reset shader to default each frame
    usingCustomShader = false; // Disable custom shader by default

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

    renderParticles(dt);
}

int l_getRunningTime(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushnumber(L, 0.0);
        return 1;
    }
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - framework->startTime;
    lua_pushnumber(L, elapsedTime);
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