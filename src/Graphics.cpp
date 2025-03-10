#include "VoltaFramework.hpp"
#include <cmath>
#include <cstring>

int l_rectangle(lua_State* L) {
    int isbool {lua_isboolean(L, 1)};
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill {lua_toboolean(L, 1) != 0};
    
    // Get position Vector2
    Vector2* position = checkVector2(L, 2);
    
    // Get size Vector2
    Vector2* size = checkVector2(L, 3);

    // Get the current window size from the framework
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_rectangle: Framework or window is null\n";
        return 0;
    }
    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    // Convert to OpenGL coordinates (-1 to 1) based on stored window size
    float left {static_cast<float>(position->x / (width / 2.0)) - 1.0f};
    float right {static_cast<float>((position->x + size->x) / (width / 2.0)) - 1.0f};
    float top {static_cast<float>(1.0 - (position->y / (height / 2.0)))};
    float bottom {static_cast<float>(1.0 - ((position->y + size->y) / (height / 2.0)))};

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
    
    // Get center Vector2
    Vector2* center = checkVector2(L, 2);
    
    // Get radius as a number
    lua_Number r {luaL_checknumber(L, 3)};

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_circle: Framework or window is null\n";
        return 0;
    }
    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    float centerX {static_cast<float>((center->x / (width / 2.0)) - 1.0)};
    float centerY {static_cast<float>(1.0 - (center->y / (height / 2.0)))};
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

int l_drawLine(lua_State* L) {
    // Check and retrieve arguments using Vector2 for start and end points
    Vector2* start = checkVector2(L, 1);
    Vector2* end = checkVector2(L, 2);
    lua_Number lineWidth {luaL_optnumber(L, 3, 1.0)}; // Optional 3rd argument, default to 1.0

    // Ensure line width is positive
    if (lineWidth <= 0) {
        luaL_argerror(L, 3, "line width must be positive");
    }

    // Get the framework instance and window size
    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawLine: Framework or window is null\n";
        return 0;
    }
    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    // Convert to OpenGL coordinates (-1 to 1) based on window size
    float startX {static_cast<float>((start->x / (width / 2.0)) - 1.0)};
    float startY {static_cast<float>(1.0 - (start->y / (height / 2.0)))};
    float endX {static_cast<float>((end->x / (width / 2.0)) - 1.0)};
    float endY {static_cast<float>(1.0 - (end->y / (height / 2.0)))};

    // Calculate the direction of the line (in NDC space)
    float dx {endX - startX};
    float dy {endY - startY};
    float length {std::sqrt(dx * dx + dy * dy)};
    if (length == 0) {
        // If the line has no length, there's nothing to draw
        return 0;
    }

    // Normalize the direction vector
    float nx {dx / length};
    float ny {dy / length};

    // Calculate the perpendicular vector (rotate 90 degrees)
    float perpX {-ny};
    float perpY {nx};

    // Calculate the half-width in NDC space
    float halfWidth {static_cast<float>(lineWidth / (width / 2.0))}; // Adjust based on window width
    // You might want to adjust based on height as well if aspect ratio matters
    // For simplicity, we use width here; tweak as needed for your use case

    // Compute the four corners of the quad
    float halfWidthPerpX {perpX * halfWidth};
    float halfWidthPerpY {perpY * halfWidth};

    // Define the four corners of the quad
    float p1x {startX - halfWidthPerpX};
    float p1y {startY - halfWidthPerpY};
    float p2x {startX + halfWidthPerpX};
    float p2y {startY + halfWidthPerpY};
    float p3x {endX + halfWidthPerpX};
    float p3y {endY + halfWidthPerpY};
    float p4x {endX - halfWidthPerpX};
    float p4y {endY - halfWidthPerpY};

    // Draw the quad using OpenGL
    glBegin(GL_QUADS);
    glVertex2f(p1x, p1y);
    glVertex2f(p2x, p2y);
    glVertex2f(p3x, p3y);
    glVertex2f(p4x, p4y);
    glEnd();

    return 0; // No return values to Lua
}

int l_drawImage(lua_State* L) {
    const char* filename {luaL_checkstring(L, 1)};
    
    // Get position Vector2
    Vector2* position = checkVector2(L, 2);
    
    // Get size Vector2
    Vector2* size = checkVector2(L, 3);

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawImage: Framework or window is null\n";
        return 0;
    }

    float width {static_cast<float>(framework->getWidth())};
    float height {static_cast<float>(framework->getHeight())};

    float left {static_cast<float>((position->x / (width / 2.0)) - 1.0)};
    float right {(static_cast<float>(position->x) + static_cast<float>(size->x)) / (width / 2.0f) - 1.0f};
    float top {static_cast<float>(1.0 - (position->y / (height / 2.0)))};
    float bottom {static_cast<float>(1.0 - ((position->y + size->y) / (height / 2.0)))};

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

int l_setColor(lua_State* L) {
    lua_Number r {luaL_checknumber(L, 1)};
    lua_Number g {luaL_checknumber(L, 2)};
    lua_Number b {luaL_checknumber(L, 3)};
    glColor3f(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
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