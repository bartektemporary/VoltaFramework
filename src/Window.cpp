#include "VoltaFramework.hpp"

// VoltaFramework.cpp (implement new methods and update Lua to use them)

std::string VoltaFramework::getWindowTitle() const {
    if (!window) {
        std::cerr << "getWindowTitle: Window is null\n";
        return "";
    }
    const char* title = glfwGetWindowTitle(window);
    return title ? std::string(title) : "";
}

void VoltaFramework::setWindowTitle(const std::string& title) {
    if (!window) {
        std::cerr << "setWindowTitle: Window is null\n";
        return;
    }
    glfwSetWindowTitle(window, title.c_str());
}

void VoltaFramework::setWindowSize(int width, int height) {
    if (!window) {
        std::cerr << "setWindowSize: Window is null\n";
        return;
    }
    width = std::max(1, width);
    height = std::max(1, height);
    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}

void VoltaFramework::getWindowSize(int& width, int& height) const {
    if (!window) {
        std::cerr << "getWindowSize: Window is null\n";
        width = 0;
        height = 0;
        return;
    }
    width = this->width;
    height = this->height;
}

void VoltaFramework::setWindowPosition(int x, int y) {
    if (!window) {
        std::cerr << "setWindowPosition: Window is null\n";
        return;
    }
    glfwSetWindowPos(window, x, y);
    this->x = x;
    this->y = y;
}

void VoltaFramework::getWindowPosition(int& x, int& y) const {
    if (!window) {
        std::cerr << "getWindowPosition: Window is null\n";
        x = 0;
        y = 0;
        return;
    }
    x = this->x;
    y = this->y;
}

void VoltaFramework::setWindowResizable(bool resizable) {
    if (!window) {
        std::cerr << "setWindowResizable: Window is null\n";
        return;
    }
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

void VoltaFramework::setFullscreen(bool enable) {
    if (!window) {
        std::cerr << "setFullscreen: Window is null\n";
        return;
    }
    if (enable) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            std::cerr << "setFullscreen: Failed to get primary monitor\n";
            return;
        }
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) {
            std::cerr << "setFullscreen: Failed to get video mode\n";
            return;
        }
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glViewport(0, 0, mode->width, mode->height);
        width = mode->width;
        height = mode->height;
    } else {
        glfwSetWindowMonitor(window, nullptr, x, y, 800, 600, 0);
        glViewport(0, 0, 800, 600);
        width = 800;
        height = 600;
    }
}

void VoltaFramework::setWindowState(const std::string& state) {
    if (!window) {
        std::cerr << "setWindowState: Window is null\n";
        return;
    }
    if (state == "normal") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwRestoreWindow(window);
        this->state = 0;
    } else if (state == "minimized") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwIconifyWindow(window);
        this->state = 1;
    } else if (state == "maximized") {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwMaximizeWindow(window);
        this->state = 2;
    } else if (state == "borderlessMaximized") {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            std::cerr << "setWindowState: Failed to get primary monitor\n";
            return;
        }
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) {
            std::cerr << "setWindowState: Failed to get video mode\n";
            return;
        }
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(window, 0, 0);
        glfwSetWindowSize(window, mode->width, mode->height);
        x = 0;
        y = 0;
        width = mode->width;
        height = mode->height;
        this->state = 3;
        glViewport(0, 0, mode->width, mode->height);
    } else {
        std::cerr << "setWindowState: Invalid state '" << state << "'\n";
    }
}

std::string VoltaFramework::getWindowState() const {
    if (!window) {
        std::cerr << "getWindowState: Window is null\n";
        return "normal";
    }
    switch (state) {
        case 0: return "normal";
        case 1: return "minimized";
        case 2: return "maximized";
        case 3: return "borderlessMaximized";
        default: return "normal";
    }
}

void VoltaFramework::setWindowIcon(const std::string& filename) {
    if (!window) {
        std::cerr << "setWindowIcon: Window is null\n";
        return;
    }
    if (filename.empty()) {
        glfwSetWindowIcon(window, 0, nullptr);
        return;
    }

    std::string fullPath = std::string("assets/") + filename;
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fullPath.c_str(), 0);
    if (format == FIF_UNKNOWN) {
        format = FreeImage_GetFIFFromFilename(fullPath.c_str());
    }
    if (format == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(format)) {
        std::cerr << "setWindowIcon: Unsupported image format: " << fullPath << "\n";
        return;
    }

    FIBITMAP* bitmap = FreeImage_Load(format, fullPath.c_str());
    if (!bitmap) {
        std::cerr << "setWindowIcon: Failed to load image: " << fullPath << "\n";
        return;
    }

    FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
    FreeImage_Unload(bitmap);
    if (!bitmap32) {
        std::cerr << "setWindowIcon: Failed to convert image to 32-bit: " << fullPath << "\n";
        return;
    }

    FIBITMAP* flippedBitmap = FreeImage_Clone(bitmap32);
    FreeImage_FlipVertical(flippedBitmap);

    unsigned int w = FreeImage_GetWidth(flippedBitmap);
    unsigned int h = FreeImage_GetHeight(flippedBitmap);
    unsigned char* pixels = FreeImage_GetBits(flippedBitmap);

    for (unsigned int i = 0; i < w * h; i++) {
        unsigned char temp = pixels[i * 4];
        pixels[i * 4] = pixels[i * 4 + 2];
        pixels[i * 4 + 2] = temp;
    }

    GLFWimage image = {static_cast<int>(w), static_cast<int>(h), pixels};
    glfwSetWindowIcon(window, 1, &image);

    FreeImage_Unload(bitmap32);
    FreeImage_Unload(flippedBitmap);
}

void VoltaFramework::setVsync(bool enable) {
    if (!window) {
        std::cerr << "setVsync: Window is null\n";
        return;
    }
    glfwSwapInterval(enable ? 1 : 0);
}

// Update Lua window functions to use C++ methods
int l_window_getTitle(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getTitle: Framework or window is null\n";
        lua_pushstring(L, "");
        return 1;
    }
    lua_pushstring(L, framework->getWindowTitle().c_str());
    return 1;
}

int l_window_setTitle(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setTitle: Framework or window is null\n";
        return 0;
    }
    const char* title = luaL_checkstring(L, 1);
    framework->setWindowTitle(title);
    return 0;
}

int l_window_setSize(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setSize: Framework or window is null\n";
        return 0;
    }
    lua_Integer width = luaL_checkinteger(L, 1);
    lua_Integer height = luaL_checkinteger(L, 2);
    framework->setWindowSize(static_cast<int>(width), static_cast<int>(height));
    return 0;
}

int l_window_getSize(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getSize: Framework or window is null\n";
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 2;
    }
    int width, height;
    framework->getWindowSize(width, height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

int l_window_setPosition(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setPosition: Framework or window is null\n";
        return 0;
    }
    lua_Integer x = luaL_checkinteger(L, 1);
    lua_Integer y = luaL_checkinteger(L, 2);
    framework->setWindowPosition(static_cast<int>(x), static_cast<int>(y));
    return 0;
}

int l_window_getPosition(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getPosition: Framework or window is null\n";
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 2;
    }
    int x, y;
    framework->getWindowPosition(x, y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

int l_window_setResizable(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setResizable: Framework or window is null\n";
        return 0;
    }
    bool enable = lua_toboolean(L, 1) != 0;
    framework->setWindowResizable(enable);
    return 0;
}

int l_window_setFullscreen(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setFullscreen: Framework or window is null\n";
        return 0;
    }
    bool enable = lua_toboolean(L, 1) != 0;
    framework->setFullscreen(enable);
    return 0;
}

int l_window_setState(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setState: Framework or window is null\n";
        return 0;
    }
    const char* stateStr = luaL_checkstring(L, 1);
    framework->setWindowState(stateStr);
    return 0;
}

int l_window_getState(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "getState: Framework or window is null\n";
        lua_pushstring(L, "normal");
        return 1;
    }
    lua_pushstring(L, framework->getWindowState().c_str());
    return 1;
}

int l_window_setIcon(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setIcon: Framework or window is null\n";
        return 0;
    }
    int nargs = lua_gettop(L);
    if (nargs == 0 || lua_isnil(L, 1)) {
        framework->setWindowIcon("");
        return 0;
    }
    const char* filename = luaL_checkstring(L, 1);
    framework->setWindowIcon(filename);
    return 0;
}

int l_window_setVsync(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "setVsync: Framework or window is null\n";
        return 0;
    }
    if (!lua_isboolean(L, 1)) {
        luaL_argerror(L, 1, "boolean expected");
        return 0;
    }
    bool enable = lua_toboolean(L, 1) != 0;
    framework->setVsync(enable);
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