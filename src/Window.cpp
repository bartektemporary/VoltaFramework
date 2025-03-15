#include "VoltaFramework.hpp"

int l_window_getTitle(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "getTitle: Framework or window is null\n";
        lua_pushstring(L, "");
        return 1;
    }
    const char* title {glfwGetWindowTitle(framework->getWindow())};
    lua_pushstring(L, title ? title : "");
    return 1;
}

int l_window_setTitle(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setTitle: Framework or window is null\n";
        return 0;
    }
    const char* title {luaL_checkstring(L, 1)};
    glfwSetWindowTitle(framework->getWindow(), title);
    return 0;
}

int l_window_setSize(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setSize: Framework or window is null\n";
        return 0;
    }
    lua_Integer width {luaL_checkinteger(L, 1)};
    lua_Integer height {luaL_checkinteger(L, 2)};
    width = std::max(1LL, width);
    height = std::max(1LL, height);
    glfwSetWindowSize(framework->getWindow(), static_cast<int>(width), static_cast<int>(height));
    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    framework->setWidth(static_cast<int>(width));
    framework->setHeight(static_cast<int>(height));
    return 0;
}

int l_window_getSize(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "getSize: Framework or window is null\n";
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 2;
    }
    int width {framework->getWidth()};
    int height {framework->getHeight()};
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

int l_window_setPosition(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setPosition: Framework or window is null\n";
        return 0;
    }
    lua_Integer x {luaL_checkinteger(L, 1)};
    lua_Integer y {luaL_checkinteger(L, 2)};
    glfwSetWindowPos(framework->getWindow(), static_cast<int>(x), static_cast<int>(y));
    framework->setX(static_cast<int>(x));
    framework->setY(static_cast<int>(y));
    return 0;
}

int l_window_getPosition(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "getPosition: Framework or window is null\n";
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 2;
    }
    int x {framework->getX()};
    int y {framework->getY()};
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

int l_window_setResizable(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setResizable: Framework or window is null\n";
        return 0;
    }
    bool enable {lua_toboolean(L, 1) != 0};
    glfwSetWindowAttrib(framework->getWindow(), GLFW_RESIZABLE, enable ? GLFW_TRUE : GLFW_FALSE);
    return 0;
}

int l_window_setFullscreen(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setFullscreen: Framework or window is null\n";
        return 0;
    }

    bool enable {lua_toboolean(L, 1) != 0};
    GLFWwindow* window {framework->getWindow()};

    if (enable) {
        GLFWmonitor* monitor {glfwGetPrimaryMonitor()};
        if (!monitor) {
            std::cerr << "setFullscreen: Failed to get primary monitor\n";
            return 0;
        }

        const GLFWvidmode* mode {glfwGetVideoMode(monitor)};
        if (!mode) {
            std::cerr << "setFullscreen: Failed to get video mode\n";
            return 0;
        }

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glViewport(0, 0, mode->width, mode->height);
        framework->setWidth(mode->width);
        framework->setHeight(mode->height);
    } else {
        glfwSetWindowMonitor(window, nullptr, framework->getX(), framework->getY(), 800, 600, 0);
        glViewport(0, 0, 800, 600);
        framework->setWidth(800);
        framework->setHeight(600);
    }

    return 0;
}

int l_window_setState(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setState: Framework or window is null\n";
        return 0;
    }
    const char* stateStr {luaL_checkstring(L, 1)};
    GLFWwindow* window {framework->getWindow()};
    std::string state {stateStr};

    if (state == "normal") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwRestoreWindow(window);
        framework->setState(0);
    } else if (state == "minimized") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwIconifyWindow(window);
        framework->setState(1);
    } else if (state == "maximized") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwMaximizeWindow(window);
        framework->setState(2);
    } else if (state == "borderlessMaximized") {
        GLFWmonitor* monitor {glfwGetPrimaryMonitor()};
        if (!monitor) {
            std::cerr << "setState: Failed to get primary monitor for borderlessMaximized state\n";
            return 0;
        }
        
        const GLFWvidmode* mode {glfwGetVideoMode(monitor)};
        if (!mode) {
            std::cerr << "setState: Failed to get video mode\n";
            return 0;
        }

        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(window, 0, 0);
        glfwSetWindowSize(window, mode->width, mode->height);
        framework->setX(0);
        framework->setY(0);
        framework->setWidth(mode->width);
        framework->setHeight(mode->height);
        framework->setState(3);
        glViewport(0, 0, mode->width, mode->height);
    } else {
        std::cerr << "setState: Invalid state '" << state << "' (use 'normal', 'minimized', 'maximized', 'borderlessMaximized')\n";
    }

    return 0;
}

int l_window_getState(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "getState: Framework or window is null\n";
        lua_pushstring(L, "normal");
        return 1;
    }
    int state {framework->getState()};
    const char* stateStr {};
    switch (state) {
        case 0: stateStr = "normal"; break;
        case 1: stateStr = "minimized"; break;
        case 2: stateStr = "maximized"; break;
        case 3: stateStr = "borderlessMaximized"; break;
        default: stateStr = "normal";
    }
    lua_pushstring(L, stateStr);
    return 1;
}

int l_window_setIcon(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setIcon: Framework or window is null\n";
        return 0;
    }

    GLFWwindow* window = framework->getWindow();
    int nargs = lua_gettop(L);

    if (nargs == 0 || lua_isnil(L, 1)) {
        glfwSetWindowIcon(window, 0, nullptr);
        return 0;
    }

    const char* filename = luaL_checkstring(L, 1);
    std::string fullPath = std::string("assets/") + filename;

    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fullPath.c_str(), 0);
    if (format == FIF_UNKNOWN) {
        format = FreeImage_GetFIFFromFilename(fullPath.c_str());
    }
    if (format == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(format)) {
        std::cerr << "setIcon: Unsupported image format: " << fullPath << std::endl;
        return 0;
    }

    FIBITMAP* bitmap = FreeImage_Load(format, fullPath.c_str());
    if (!bitmap) {
        std::cerr << "setIcon: Failed to load image: " << fullPath << std::endl;
        return 0;
    }

    FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
    FreeImage_Unload(bitmap);
    if (!bitmap32) {
        std::cerr << "setIcon: Failed to convert image to 32-bit: " << fullPath << std::endl;
        return 0;
    }

    FIBITMAP* flippedBitmap = FreeImage_Clone(bitmap32);
    FreeImage_FlipVertical(flippedBitmap);

    unsigned int width = FreeImage_GetWidth(flippedBitmap);
    unsigned int height = FreeImage_GetHeight(flippedBitmap);
    unsigned char* pixels = FreeImage_GetBits(flippedBitmap);

    for (unsigned int i = 0; i < width * height; i++) {
        unsigned char temp = pixels[i * 4];
        pixels[i * 4] = pixels[i * 4 + 2];
        pixels[i * 4 + 2] = temp;
    }

    GLFWimage image = {static_cast<int>(width), static_cast<int>(height), pixels};
    glfwSetWindowIcon(window, 1, &image);

    FreeImage_Unload(bitmap32);
    FreeImage_Unload(flippedBitmap);

    return 0;
}

int l_window_setVsync(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "setVsync: Framework or window is null\n";
        return 0;
    }
    
    if (!lua_isboolean(L, 1)) {
        luaL_argerror(L, 1, "boolean expected");
        return 0;
    }
    
    bool enable {lua_toboolean(L, 1) != 0};
    glfwSwapInterval(enable ? 1 : 0);
    
    return 0;
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
            g_frameworkInstance->state = 2;
        } else if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            g_frameworkInstance->state = 1;
        } else {
            g_frameworkInstance->state = 0;
        }
    }
}