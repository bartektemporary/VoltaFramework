#include "VoltaFramework.hpp"
#include <cmath>
#include <cstring>

int l_rectangle(lua_State* L) {
    int isbool {lua_isboolean(L, 1)};
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill {lua_toboolean(L, 1) != 0};
    lua_Number x {luaL_checknumber(L, 2)};
    lua_Number y {luaL_checknumber(L, 3)};
    lua_Number w {luaL_checknumber(L, 4)};
    lua_Number h {luaL_checknumber(L, 5)};

    // Get the current window size from the framework
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_rectangle: Framework or window is null\n";
        return 0;
    }
    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    // Convert to OpenGL coordinates (-1 to 1) based on stored window size
    float left {static_cast<float>(x / (width / 2.0)) - 1.0f};
    float right {static_cast<float>((x + w) / (width / 2.0)) - 1.0f};
    float top {static_cast<float>(1.0 - (y / (height / 2.0)))};
    float bottom {static_cast<float>(1.0 - ((y + h) / (height / 2.0)))};

    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);
    glEnd();

    return 0;
}

int l_circle(lua_State* L) {
    int isbool {lua_isboolean(L, 1)};
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill {lua_toboolean(L, 1) != 0};
    lua_Number cx {luaL_checknumber(L, 2)};
    lua_Number cy {luaL_checknumber(L, 3)};
    lua_Number r {luaL_checknumber(L, 4)};

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_circle: Framework or window is null\n";
        return 0;
    }
    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    float centerX {static_cast<float>((cx / (width / 2.0)) - 1.0)};
    float centerY {static_cast<float>(1.0 - (cy / (height / 2.0)))};
    float radiusX {static_cast<float>(r / (width / 2.0))};
    float radiusY {static_cast<float>(r / (height / 2.0))};

    if (fill) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int ii = 0; ii <= 35; ii++) {
            float theta {static_cast<float>(2.0 * 3.1415926 * float(ii) / 35.0)};
            float x {radiusX * cosf(theta)};
            float y {radiusY * sinf(theta)};
            glVertex2f(centerX + x, centerY + y);
        }
    } else {
        glBegin(GL_LINE_LOOP);
        for (int ii = 0; ii < 35; ii++) {
            float theta {static_cast<float>(2.0 * 3.1415926 * float(ii) / 35.0)};
            float x {radiusX * cosf(theta)};
            float y {radiusY * sinf(theta)};
            glVertex2f(centerX + x, centerY + y);
        }
    }
    glEnd();

    return 0;
}

int l_setColor(lua_State* L) {
    lua_Number r {luaL_checknumber(L, 1)};
    lua_Number g {luaL_checknumber(L, 2)};
    lua_Number b {luaL_checknumber(L, 3)};
    glColor3f(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
    return 0;
}

int l_drawImage(lua_State* L) {
    const char* filename {luaL_checkstring(L, 1)};
    lua_Number x {luaL_checknumber(L, 2)};
    lua_Number y {luaL_checknumber(L, 3)};
    lua_Number w {luaL_checknumber(L, 4)};
    lua_Number h {luaL_checknumber(L, 5)};

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawImage: Framework or window is null\n";
        return 0;
    }

    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    float left {static_cast<float>((x / (width / 2.0)) - 1.0)};
    float right {(static_cast<float>(x) + static_cast<float>(w)) / (width / 2.0f) - 1.0f};
    float top {static_cast<float>(1.0 - (y / (height / 2.0)))};
    float bottom {static_cast<float>(1.0 - ((y + h) / (height / 2.0)))};

    std::string fullPath {std::string("assets/") + filename};
    GLuint textureID {framework->loadTexture(fullPath)};
    if (textureID == 0) {
        return 0;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(left, bottom);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(right, bottom);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(right, top);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(left, top);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    return 0;
}

int l_setFilter(lua_State* L) {
    const char* mode {luaL_checkstring(L, 1)}; // Expect "nearest" or "linear"
    VoltaFramework* framework {getFramework(L)};
    if (!framework) {
        std::cerr << "l_setFilter: Framework is null\n";
        return 0;
    }

    if (strcmp(mode, "nearest") == 0) {
        framework->setFilterMode(GL_NEAREST);
    } else if (strcmp(mode, "linear") == 0) {
        framework->setFilterMode(GL_LINEAR);
    } else {
        luaL_argerror(L, 1, "expected 'nearest' or 'linear'");
    }

    // Optionally update existing textures (see note below)
    return 0;
}