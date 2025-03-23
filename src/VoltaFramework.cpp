#include "VoltaFramework.hpp"
#include <fstream>
#include <cstring>
#include "Maps.hpp"

VoltaFramework* g_frameworkInstance {nullptr};

VoltaFramework::VoltaFramework() : 
    L{nullptr}, 
    window{nullptr}, 
    width{800}, 
    height{600}, 
    x{100}, 
    y{100}, 
    startTime{0.0},
    usingCustomShader(false),
    ftLibrary{nullptr},
    ftFace{nullptr},
    textVAO{0},
    textVBO{0},
    textureVAO{0},
    textureVBO{0},
    particleVAO{0},
    particleVBO{0}
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

    currentColor[0] = 1.0f;
    currentColor[1] = 1.0f;
    currentColor[2] = 1.0f;

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
    for (auto& pair : fontCache) {
        FT_Done_Face(pair.second);
    }
    fontCache.clear();
    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
    }
    ma_engine_uninit(&engine);
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

std::string VoltaFramework::loadFile(const std::string& filename, bool asText) {
    fs::path fullPath;

    // Check if the filename contains a separator
    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        fullPath = fs::path(filename); // Use as-is if it contains separators
    } else {
        fullPath = fs::path("assets") / filename; // Prepend "assets/" for simple filenames
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
    loadLuaScript("scripts/main.lua");

    // Get the 'volta' table
    lua_getglobal(L, "volta");
    if (lua_istable(L, -1)) {
        // Get the 'init' function from the 'volta' table
        lua_getfield(L, -1, "init");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                std::cerr << "Init Error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        } else {
            std::cerr << "Warning: volta.init is not a function or not defined\n";
            lua_pop(L, 1); // Pop the non-function value
        }
    } else {
        std::cerr << "Error: 'volta' table not found in Lua environment\n";
    }
    lua_pop(L, 1); // Pop the 'volta' table

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

    currentColor[0] = 1.0f;
    currentColor[1] = 1.0f;
    currentColor[2] = 1.0f;
    currentShaderName = "";
    usingCustomShader = false;

    lua_getglobal(L, "volta");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "update");
        if (lua_isfunction(L, -1)) {
            lua_pushnumber(L, dt);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                std::cerr << "Update Error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1);
        }
    } else {
        std::cerr << "Error: 'volta' table not found in Lua environment\n";
    }
    lua_pop(L, 1);

    renderParticles(dt); // Assumed 2D for now; adjust if 3D particles are needed
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