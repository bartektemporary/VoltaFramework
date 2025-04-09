#include "VoltaFramework.hpp"
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstring>
#include <fstream>
#include <sstream>

void VoltaFramework::windowToGLCoords(float winX, float winY, float* glX, float* glY) {
    *glX = (winX / (width / 2.0f)) - 1.0f;
    *glY = (winY / (height / 2.0f)) - 1.0f;  // Changed from 1.0f - (winY / (height / 2.0f))
}

void VoltaFramework::drawRectangle(bool fill, const Vector2& position, const Vector2& size, float rotation) {
    Vector2 adjustedPosition = position;

    if (positionMode == PositionMode::World && currentCamera != nullptr) {
        Vector2 cameraPos = currentCamera->getPosition();
        adjustedPosition.x = position.x - cameraPos.x + width / 2.0f;
        adjustedPosition.y = position.y - cameraPos.y + height / 2.0f;

        float halfW = size.x / 2.0f;
        float halfH = size.y / 2.0f;
        Rect bounds(position.x - halfW, position.x + halfW, position.y - halfH, position.y + halfH);
        if (!isRectInView(bounds)) {
            return;
        }
    }

    Vector2 vertices[4];
    float hw = size.x / 2.0f;
    float hh = size.y / 2.0f;

    vertices[0].x = -hw; vertices[0].y = -hh;
    vertices[1].x = hw;  vertices[1].y = -hh;
    vertices[2].x = hw;  vertices[2].y = hh;
    vertices[3].x = -hw; vertices[3].y = hh;

    if (rotation != 0) {
        float cosR = cosf(rotation * M_PI / 180.0f); // Convert degrees to radians
        float sinR = sinf(rotation * M_PI / 180.0f);
        for (int i = 0; i < 4; i++) {
            float x = vertices[i].x;
            float y = vertices[i].y;
            vertices[i].x = x * cosR - y * sinR;
            vertices[i].y = x * sinR + y * cosR;
        }
    }

    for (int i = 0; i < 4; i++) {
        vertices[i].x += adjustedPosition.x;
        vertices[i].y += adjustedPosition.y;
        float glX, glY;
        windowToGLCoords(vertices[i].x, vertices[i].y, &glX, &glY);
        vertices[i].x = glX;
        vertices[i].y = glY;
    }

    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * 4, vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shape2DShaderProgram);
    glUniform3f(shape2DColorUniform, currentColor.r, currentColor.g, currentColor.b);

    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    glBindVertexArray(0);
}

int l_rectangle(lua_State* L) {
    int isbool = lua_isboolean(L, 1);
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill = lua_toboolean(L, 1) != 0;
    
    Vector2* position = checkVector2(L, 2);
    Vector2* size = checkVector2(L, 3);
    lua_Number rotation = luaL_optnumber(L, 4, 0.0);

    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_rectangle: Framework or window is null\n";
        return 0;
    }
    
    framework->drawRectangle(fill, *position, *size, static_cast<float>(rotation));
    return 0;
}

void VoltaFramework::drawCircle(bool fill, const Vector2& position, float radius) {
    Vector2 adjustedPosition = position;

    if (positionMode == PositionMode::World && currentCamera != nullptr) {
        Vector2 cameraPos = currentCamera->getPosition();
        adjustedPosition.x = position.x - cameraPos.x + width / 2.0f;
        adjustedPosition.y = position.y - cameraPos.y + height / 2.0f;

        Rect bounds(position.x - radius, position.x + radius, position.y - radius, position.y + radius);
        if (!isRectInView(bounds)) {
            return;
        }
    }

    const int segments = 32;
    Vector2 vertices[segments];
    for (int i = 0; i < segments; i++) {
        float angle = (float)i / segments * 2.0f * M_PI;
        vertices[i].x = cosf(angle) * radius + adjustedPosition.x;
        vertices[i].y = sinf(angle) * radius + adjustedPosition.y;
        float glX, glY;
        windowToGLCoords(vertices[i].x, vertices[i].y, &glX, &glY);
        vertices[i].x = glX;
        vertices[i].y = glY;
    }

    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * segments, vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shape2DShaderProgram);
    glUniform3f(shape2DColorUniform, currentColor.r, currentColor.g, currentColor.b);

    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, segments);
    }
    glBindVertexArray(0);
}

int l_circle(lua_State* L) {
    int isbool = lua_isboolean(L, 1);
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill = lua_toboolean(L, 1) != 0;
    
    Vector2* center = checkVector2(L, 2);
    lua_Number r = luaL_checknumber(L, 3);

    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_circle: Framework or window is null\n";
        return 0;
    }
    
    framework->drawCircle(fill, *center, static_cast<float>(r));
    return 0;
}

void VoltaFramework::drawLine(const Vector2& start, const Vector2& end, float lineWidth) {
    Vector2 adjustedStart = start;
    Vector2 adjustedEnd = end;

    if (positionMode == PositionMode::World && currentCamera != nullptr) {
        Vector2 cameraPos = currentCamera->getPosition();
        adjustedStart.x = start.x - cameraPos.x + width / 2.0f;
        adjustedStart.y = start.y - cameraPos.y + height / 2.0f;
        adjustedEnd.x = end.x - cameraPos.x + width / 2.0f;
        adjustedEnd.y = end.y - cameraPos.y + height / 2.0f;

        float halfWidth = lineWidth / 2.0f;
        float left = std::min(start.x, end.x) - halfWidth;
        float right = std::max(start.x, end.x) + halfWidth;
        float bottom = std::min(start.y, end.y) - halfWidth;
        float top = std::max(start.y, end.y) + halfWidth;
        Rect bounds(left, right, bottom, top);
        if (!isRectInView(bounds)) {
            return;
        }
    }

    float dx = adjustedEnd.x - adjustedStart.x;
    float dy = adjustedEnd.y - adjustedStart.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.001f) {
        return;
    }

    float nx = dx / length;
    float ny = dy / length;
    float perpX = -ny;
    float perpY = nx;

    float halfWidth = lineWidth * 2.5f; // Scale up to 5 for lineWidth=2
    float vertices[] = {
        adjustedStart.x - perpX * halfWidth, adjustedStart.y - perpY * halfWidth, // Bottom-left
        adjustedStart.x + perpX * halfWidth, adjustedStart.y + perpY * halfWidth, // Top-left
        adjustedEnd.x + perpX * halfWidth, adjustedEnd.y + perpY * halfWidth,     // Top-right
        adjustedEnd.x - perpX * halfWidth, adjustedEnd.y - perpY * halfWidth      // Bottom-right
    };

    float glVertices[8];
    for (int i = 0; i < 4; i++) {
        windowToGLCoords(vertices[i * 2], vertices[i * 2 + 1], &glVertices[i * 2], &glVertices[i * 2 + 1]);
    }

    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glVertices), glVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shape2DShaderProgram);
    glUniform3f(shape2DColorUniform, currentColor.r, currentColor.g, currentColor.b);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}


int l_drawLine(lua_State* L) {
    Vector2* start = checkVector2(L, 1);
    Vector2* end = checkVector2(L, 2);
    lua_Number lineWidth = luaL_optnumber(L, 3, 1.0);

    if (lineWidth <= 0) {
        luaL_argerror(L, 3, "line width must be positive");
    }

    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawLine: Framework or window is null\n";
        return 0;
    }
    
    framework->drawLine(*start, *end, static_cast<float>(lineWidth));
    return 0;
}

void VoltaFramework::drawImage(const std::string& filename, const Vector2& position, const Vector2& size, float rotation) {
    Vector2 adjustedPosition = position;
    if (positionMode == PositionMode::World && currentCamera != nullptr) {
        Vector2 cameraPos = currentCamera->getPosition();
        adjustedPosition.x = position.x - cameraPos.x + width / 2.0f;
        adjustedPosition.y = position.y - cameraPos.y + height / 2.0f;

        float halfW = size.x / 2.0f;
        float halfH = size.y / 2.0f;
        Rect bounds(position.x - halfW, position.x + halfW, position.y - halfH, position.y + halfH);
        if (!isRectInView(bounds)) return;
    }

    float halfWidth = size.x / 2.0f;
    float halfHeight = size.y / 2.0f;
    float rad = -rotation * M_PI / 180.0f;
    float cosR = cosf(rad);
    float sinR = sinf(rad);

    std::vector<float> rotatedVertices;
    float corners[8] = {
        -halfWidth, halfHeight,
        halfWidth, halfHeight,
        halfWidth, -halfHeight,
        -halfWidth, -halfHeight
    };

    for (int i = 0; i < 4; i++) {
        float x = corners[i * 2];
        float y = corners[i * 2 + 1];
        float rotatedX = x * cosR - y * sinR;
        float rotatedY = x * sinR + y * cosR;
        float glX, glY;
        windowToGLCoords(adjustedPosition.x + rotatedX, adjustedPosition.y + rotatedY, &glX, &glY);
        rotatedVertices.push_back(glX);
        rotatedVertices.push_back(glY);
    }

    float vertices[16] = {
        rotatedVertices[0], rotatedVertices[1], 0.0f, 1.0f,
        rotatedVertices[2], rotatedVertices[3], 1.0f, 1.0f,
        rotatedVertices[4], rotatedVertices[5], 1.0f, 0.0f,
        rotatedVertices[6], rotatedVertices[7], 0.0f, 0.0f
    };

    std::string fullPath = std::string("assets/") + filename;
    GLuint textureID = loadTexture(fullPath);
    if (textureID == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(imageShaderProgram);
    glUniform3f(imageColorUniform, currentColor.r, currentColor.g, currentColor.b);
    glUniform1i(imageTextureUniform, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Explicitly rebind EBO and set vertex attributes
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape2DEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

int l_drawImage(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    Vector2* position = static_cast<Vector2*>(luaL_checkudata(L, 2, "Vector2"));
    Vector2* size = static_cast<Vector2*>(luaL_checkudata(L, 3, "Vector2"));
    lua_Number rotation = luaL_optnumber(L, 4, 0.0);

    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawImage: Framework or window is null\n";
        return 0;
    }

    framework->drawImage(filename, *position, *size, static_cast<float>(rotation));
    return 0;
}

// Set Color (overload for Color struct)
void VoltaFramework::setColor(const Color& color) {
    currentColor = color;
}

// Ensure setColor(float, float, float) exists (if not already present)
void VoltaFramework::setColor(float r, float g, float b) {
    currentColor = Color(r, g, b);
}

int l_setColor(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "l_setColor: Framework is null\n";
        return 0;
    }

    if (lua_isuserdata(L, 1)) {
        Color* color = checkColor(L, 1);
        framework->setColor(*color);
    } else {
        lua_Number r = luaL_checknumber(L, 1);
        lua_Number g = luaL_checknumber(L, 2);
        lua_Number b = luaL_checknumber(L, 3);
        framework->setColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
    }
    return 0;
}

void VoltaFramework::setFilterMode(GLenum mode) {
    if (mode != GL_NEAREST && mode != GL_LINEAR) {
        std::cerr << "Invalid filter mode. Use GL_NEAREST or GL_LINEAR.\n";
        return;
    }
    filterMode = mode;

    // Optionally update existing textures in the cache
    for (auto& pair : textureCache) {
        glBindTexture(GL_TEXTURE_2D, pair.second);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

int l_setFilter(lua_State* L) {
    const char* mode {luaL_checkstring(L, 1)};
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
    return 0;
}

int l_setCustomShader(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* shaderName = luaL_checkstring(L, 1);
    bool success;

    if (lua_isstring(L, 2) && lua_isstring(L, 3)) {
        const char* vertexSource = luaL_checkstring(L, 2);
        const char* fragmentSource = luaL_checkstring(L, 3);
        success = framework->createCustomShader(shaderName, vertexSource, fragmentSource);
    }
    else if (lua_istable(L, 2) && lua_gettop(L) == 2) {
        lua_getfield(L, 2, "vertex");
        lua_getfield(L, 2, "fragment");
        
        if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
            luaL_argerror(L, 2, "table must contain 'vertex' and 'fragment' string fields");
            return 0;
        }
        
        const char* vertexFile = lua_tostring(L, -2);
        const char* fragmentFile = lua_tostring(L, -1);
        success = framework->createCustomShaderFromFiles(shaderName, vertexFile, fragmentFile);
        
        lua_pop(L, 2);
    }
    else {
        luaL_argerror(L, 2, "expected two strings or a table with 'vertex' and 'fragment' fields");
        return 0;
    }

    lua_pushboolean(L, success);
    return 1;
}

int l_setShader(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        return 0;
    }
    
    const char* shaderName = luaL_checkstring(L, 1);
    framework->setShader(shaderName);
    framework->useCustomShader(true); // Automatically enable custom shader when setting
    return 0;
}   

int l_useCustomShader(lua_State* L) {
    bool use = lua_toboolean(L, 1);
    
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        return 0;
    }

    framework->useCustomShader(use);
    return 0;
}


int l_clearCustomShader(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        return 0;
    }

    const char* shaderName = luaL_checkstring(L, 1);
    framework->clearCustomShader(shaderName);
    return 0;
}

int l_setCustomShaderUniform(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "l_setCustomShaderUniform: Framework is null\n";
        return 0;
    }
    
    if (lua_isnumber(L, 2)) {
        float value = static_cast<float>(luaL_checknumber(L, 2));
        framework->setShaderUniform(name, value);
    }
    else if (lua_isuserdata(L, 2)) {
        Vector2* vec = checkVector2(L, 2);
        if (vec) {
            framework->setShaderUniform(name, *vec);
        } else {
            luaL_argerror(L, 2, "Vector2 expected (invalid userdata)");
        }
    } else {
        luaL_argerror(L, 2, "number or Vector2 expected");
    }
    return 0;
}

int l_drawText(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Vector2* position = static_cast<Vector2*>(luaL_checkudata(L, 2, "Vector2"));
    lua_Number scale = luaL_optnumber(L, 3, 1.0);

    VoltaFramework* framework = getFramework(L);
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawText: Framework or window is null\n";
        return 0;
    }

    framework->drawText(text, position->x, position->y, static_cast<float>(scale));
    return 0;
}

int l_loadFont(lua_State* L) {
    const char* fontPath = luaL_checkstring(L, 1);
    lua_Number fontSize = luaL_checknumber(L, 2);

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "l_loadFont: Framework is null\n";
        return 0;
    }

    framework->loadFont(fontPath, static_cast<unsigned int>(fontSize));
    return 0;
}

int l_setFont(lua_State* L) {
    const char* fontPath = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "l_setFont: Framework is null\n";
        return 0;
    }
    framework->setFont(fontPath);
    return 0;
}

int l_setPositionMode(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }

    const char* mode = luaL_checkstring(L, 1);
    if (strcmp(mode, "screen") == 0) {
        framework->setPositionMode(VoltaFramework::PositionMode::Screen);
    } else if (strcmp(mode, "world") == 0) {
        framework->setPositionMode(VoltaFramework::PositionMode::World);
    } else {
        luaL_error(L, "Invalid position mode: %s (expected 'screen' or 'world')", mode);
        return 0;
    }

    return 0;
}

GLuint VoltaFramework::loadTexture(const std::string& filename) {
    std::string fullPath = loadFile(filename);
    if (fullPath.empty()) {
        return 0;
    }

    auto it = textureCache.find(fullPath);
    if (it != textureCache.end()) {
        return it->second;
    }

    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fullPath.c_str(), 0);
    if (format == FIF_UNKNOWN) {
        format = FreeImage_GetFIFFromFilename(fullPath.c_str());
        if (format == FIF_UNKNOWN) {
            std::cerr << "Failed to load texture: " << fullPath << " - Unknown file format" << std::endl;
            return 0;
        }
    }
    FIBITMAP* bitmap = FreeImage_Load(format, fullPath.c_str());
    if (!bitmap) {
        std::cerr << "Failed to load texture: " << fullPath << " - File not found or invalid" << std::endl;
        return 0;
    }

    FIBITMAP* converted = FreeImage_ConvertTo32Bits(bitmap);
    FreeImage_Unload(bitmap);
    BYTE* pixels = FreeImage_GetBits(converted);
    int width = FreeImage_GetWidth(converted);
    int height = FreeImage_GetHeight(converted);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glGenerateMipmap(GL_TEXTURE_2D);

    FreeImage_Unload(converted);
    textureCache[fullPath] = texture;
    return texture;
}

bool VoltaFramework::createCustomShader(const std::string& shaderName, const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint newProgram = createShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
    if (newProgram == 0) {
        std::cerr << "Failed to create custom shader program: " << shaderName << "\n";
        return false;
    }

    ShaderProgram shader;
    shader.program = newProgram;
    shader.colorUniform = glGetUniformLocation(newProgram, "uColor");
    shader.useTextureUniform = glGetUniformLocation(newProgram, "uUseTexture");
    shader.textureUniform = glGetUniformLocation(newProgram, "uTexture");

    customShaders[shaderName] = shader;
    return true;
}

void VoltaFramework::setShader(const std::string& shaderName) {
    if (customShaders.find(shaderName) != customShaders.end()) {
        currentShaderName = shaderName;
    } else {
        std::cerr << "Shader not found: " << shaderName << "\n";
        currentShaderName = "";
        usingCustomShader = false; // Disable if shader doesn't exist
    }
}

void VoltaFramework::useCustomShader(bool use) {
    usingCustomShader = use && !currentShaderName.empty();
}

void VoltaFramework::clearCustomShader(const std::string& shaderName) {
    auto it = customShaders.find(shaderName);
    if (it != customShaders.end()) {
        glDeleteProgram(it->second.program);
        customShaders.erase(it);
        if (currentShaderName == shaderName) {
            currentShaderName = "";
            usingCustomShader = false;
        }
    }
}

void VoltaFramework::clearAllCustomShaders() {
    for (auto& pair : customShaders) {
        glDeleteProgram(pair.second.program);
    }
    customShaders.clear();
    currentShaderName = "";
    usingCustomShader = false;
}

bool VoltaFramework::hasShader(const std::string& shaderName) const {
    return customShaders.find(shaderName) != customShaders.end();
}

// Text Vertex Shader (same for both, as vertex processing is identical)
const char* textVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Text Fragment Shader (for GL_RED font textures)
const char* textFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform vec3 uColor;
    uniform sampler2D uTexture;
    void main() {
        float alpha = texture(uTexture, TexCoord).r; // Use red channel as alpha
        FragColor = vec4(uColor, alpha); // Apply color with texture alpha
    }
)";

// Image Vertex Shader (same as text)
const char* imageVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Image Fragment Shader (for GL_RGBA textures)
const char* imageFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform vec3 uColor;
    uniform sampler2D uTexture;
    void main() {
        vec4 texColor = texture(uTexture, TexCoord);
        FragColor = vec4(texColor.rgb * uColor, texColor.a); // Use texture RGB and alpha
    }
)";

// Existing shape fragment shader (for shapes without textures)
const char* shapeFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 uColor;
    void main() {
        FragColor = vec4(uColor, 1.0);
    }
)";

const char* shapeVertexShaderSource2D = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* shapeVertexShaderSource3D = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

GLuint VoltaFramework::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint VoltaFramework::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) return 0;
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

bool VoltaFramework::createCustomShaderFromFiles(const std::string& shaderName, const std::string& vertexFile, const std::string& fragmentFile) {
    std::string vertexSource = loadFile(vertexFile, true);
    std::string fragmentSource = loadFile(fragmentFile, true);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to load shader files: " << vertexFile << " or " << fragmentFile << "\n";
        return false;
    }

    return createCustomShader(shaderName, vertexSource, fragmentSource);
}

void VoltaFramework::setShaderUniform(const std::string& name, float value) {
    if (usingCustomShader && !currentShaderName.empty()) {
        GLuint program = customShaders[currentShaderName].program;
        glUseProgram(program);
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUniform1f(location, value);
        } else {
            std::cerr << "Uniform '" << name << "' not found in shader '" << currentShaderName << "'\n";
        }
    } else {
        std::cerr << "No custom shader active for uniform '" << name << "'\n";
    }
}

void VoltaFramework::setShaderUniform(const std::string& name, const Vector2& value) {
    if (usingCustomShader && !currentShaderName.empty()) {
        GLuint program = customShaders[currentShaderName].program;
        glUseProgram(program); // Ensure the shader is active
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUniform2f(location, value.x, value.y);
        } else {
            std::cerr << "Uniform '" << name << "' not found in shader '" << currentShaderName << "'\n";
        }
    } else {
        std::cerr << "No custom shader active for uniform '" << name << "'\n";
    }
}

void VoltaFramework::loadFont(const std::string& fontPath, unsigned int fontSize) {
    if (!ftLibrary) {
        std::cerr << "FreeType not initialized, cannot load font: " << fontPath << "\n";
        return;
    }

    std::string fullPath = loadFile(fontPath);
    if (fullPath.empty()) {
        return;
    }

    if (fontCache.find(fontPath) != fontCache.end()) {
        return;
    }

    FT_Face newFace;
    if (FT_New_Face(ftLibrary, fullPath.c_str(), 0, &newFace)) {
        std::cerr << "ERROR: Failed to load font " << fontPath << "\n";
        return;
    }

    FT_Set_Pixel_Sizes(newFace, 0, fontSize);
    fontCache[fontPath] = newFace;

    if (!ftFace) {
        setFont(fontPath);
    }
}

void VoltaFramework::setFont(const std::string& fontPath) {
    auto it = fontCache.find(fontPath);
    if (it == fontCache.end()) {
        std::cerr << "Font '" << fontPath << "' not found. Load it first with loadFont.\n";
        return;
    }

    if (ftFace != it->second) {
        ftFace = it->second;
        for (auto& pair : characters) {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR: Failed to load glyph '" << c << "'\n";
                continue;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED, // Internal format
                ftFace->glyph->bitmap.width,
                ftFace->glyph->bitmap.rows,
                0,
                GL_RED, // Format
                GL_UNSIGNED_BYTE,
                ftFace->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                Vector2{(float)ftFace->glyph->bitmap.width, (float)ftFace->glyph->bitmap.rows},
                Vector2{(float)ftFace->glyph->bitmap_left, (float)ftFace->glyph->bitmap_top},
                (unsigned int)(ftFace->glyph->advance.x >> 6)
            };
            characters[c] = character;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void VoltaFramework::drawText(const std::string& text, float x, float y, float scale) {
    if (!ftFace || characters.empty()) return;

    Vector2 adjustedPosition(x, y);
    if (positionMode == PositionMode::World && currentCamera != nullptr) {
        Vector2 cameraPos = currentCamera->getPosition();
        adjustedPosition.x = x - cameraPos.x + width / 2.0f;
        adjustedPosition.y = y - cameraPos.y + height / 2.0f;

        // Calculate exact bounds
        float minX = x, maxX = x;
        float minY = y, maxY = y;
        float cursorX = x;
        for (char c : text) {
            if (c == '\n') {
                cursorX = x;
                minY -= (characters['A'].size.y + 10) * scale;
                continue;
            }
            auto it = characters.find(c);
            if (it == characters.end()) continue;
            Character& ch = it->second;
            float left = cursorX + ch.bearing.x * scale;
            float right = left + ch.size.x * scale;
            float bottom = y - (ch.size.y - ch.bearing.y) * scale;
            float top = bottom + ch.size.y * scale;
            minX = std::min(minX, left);
            maxX = std::max(maxX, right);
            minY = std::min(minY, bottom);
            maxY = std::max(maxY, top);
            cursorX += ch.advance * scale;
        }
        Rect bounds(minX, maxX, minY, maxY);
        if (!isRectInView(bounds)) return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(textShaderProgram);
    glUniform3f(shape2DColorUniform, currentColor.r, currentColor.g, currentColor.b);
    glUniform1i(textTextureUniform, 0);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    float cursorX = adjustedPosition.x;
    float cursorY = adjustedPosition.y;
    for (char c : text) {
        if (c == '\n') {
            cursorX = adjustedPosition.x;
            cursorY -= (characters['A'].size.y + 10) * scale;
            continue;
        }
        auto it = characters.find(c);
        if (it == characters.end()) continue;

        Character& ch = it->second;
        float xpos, ypos;
        windowToGLCoords(cursorX + ch.bearing.x * scale, cursorY - (ch.size.y - ch.bearing.y) * scale, &xpos, &ypos);
        float w = ch.size.x * scale * 2.0f / width;
        float h = ch.size.y * scale * 2.0f / height;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        cursorX += ch.advance * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND); // Reset blending
}

void VoltaFramework::initOpenGL() {
    // Initialize 2D rendering resources
    initOpenGL2D();

    // Initialize 3D rendering resources
    initOpenGL3D();

    // Initialize image rendering (shared between 2D and 3D)
    imageShaderProgram = createShaderProgram(imageVertexShaderSource, imageFragmentShaderSource);
    if (imageShaderProgram == 0) {
        std::cerr << "Failed to create image shader program\n";
        return;
    }
    imageColorUniform = glGetUniformLocation(imageShaderProgram, "uColor");
    imageTextureUniform = glGetUniformLocation(imageShaderProgram, "uTexture");

    // Texture VAO (for images)
    glGenVertexArrays(1, &textureVAO);
    glGenBuffers(1, &textureVBO);
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape2DEBO); // Reuse 2D EBO for simplicity
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Initialize text rendering
    textShaderProgram = createShaderProgram(textVertexShaderSource, textFragmentShaderSource);
    if (textShaderProgram == 0) {
        std::cerr << "Failed to create text shader program\n";
        return;
    }
    textColorUniform = glGetUniformLocation(textShaderProgram, "uColor");
    textTextureUniform = glGetUniformLocation(textShaderProgram, "uTexture");

    // Text VAO
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Particle VAO (assumed 2D for now)
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glViewport(0, 0, width, height);
}

void VoltaFramework::initOpenGL2D() {
    shape2DShaderProgram = createShaderProgram(shapeVertexShaderSource2D, shapeFragmentShaderSource);
    shape2DColorUniform = glGetUniformLocation(shape2DShaderProgram, "uColor");
    glGenVertexArrays(1, &shape2DVAO);
    glGenBuffers(1, &shape2DVBO);
    glGenBuffers(1, &shape2DEBO);
    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * 64, nullptr, GL_DYNAMIC_DRAW); // Increased for safety
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Initialize particle VAO/VBO
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 1024 * 4, nullptr, GL_DYNAMIC_DRAW); // Large initial buffer
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    projection2D.setIdentity();
    float left = 0.0f, right = (float)width, bottom = 0.0f, top = (float)height;
    projection2D.m[0] = 2.0f / (right - left);
    projection2D.m[5] = 2.0f / (top - bottom);
    projection2D.m[12] = -(right + left) / (right - left);
    projection2D.m[13] = -(top + bottom) / (top - bottom);
    setCamera2D(nullptr);
}

void VoltaFramework::initOpenGL3D() {
    // Enable depth testing and face culling for 3D
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // 3D Shape Shader
    shape3DShaderProgram = createShaderProgram(shapeVertexShaderSource3D, shapeFragmentShaderSource);
    if (shape3DShaderProgram == 0) {
        std::cerr << "Failed to create 3D shape shader program\n";
        return;
    }
    shape3DColorUniform = glGetUniformLocation(shape3DShaderProgram, "uColor");
    shape3DProjectionUniform = glGetUniformLocation(shape3DShaderProgram, "projection");
    shape3DViewUniform = glGetUniformLocation(shape3DShaderProgram, "view");
    shape3DModelUniform = glGetUniformLocation(shape3DShaderProgram, "model");

    // 3D VAO
    glGenVertexArrays(1, &shape3DVAO);
    glGenBuffers(1, &shape3DVBO);
    glGenBuffers(1, &shape3DEBO);
    glBindVertexArray(shape3DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape3DVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape3DEBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Perspective projection and view for 3D
    setPerspective(projection3D, 45.0f * M_PI / 180.0f, (float)width / (float)height, 0.1f, 100.0f);
    setLookAt(view3D, {0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    model.setIdentity();
}

void VoltaFramework::drawCube(const Vector3& position, const Vector3& size, const Vector3& rotation) {
    // Define cube vertices (centered at origin)
    float halfX = size.x / 2.0f;
    float halfY = size.y / 2.0f;
    float halfZ = size.z / 2.0f;
    float vertices[] = {
        -halfX, -halfY,  halfZ,  // 0
         halfX, -halfY,  halfZ,  // 1
         halfX,  halfY,  halfZ,  // 2
        -halfX,  halfY,  halfZ,  // 3
        -halfX, -halfY, -halfZ,  // 4
         halfX, -halfY, -halfZ,  // 5
         halfX,  halfY, -halfZ,  // 6
        -halfX,  halfY, -halfZ   // 7
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,  // Front
        1, 5, 6,  6, 2, 1,  // Right
        5, 4, 7,  7, 6, 5,  // Back
        4, 0, 3,  3, 7, 4,  // Left
        3, 2, 6,  6, 7, 3,  // Top
        0, 4, 5,  5, 1, 0   // Bottom
    };

    // Convert rotations from degrees to radians
    float radX = rotation.x * M_PI / 180.0f;
    float radY = rotation.y * M_PI / 180.0f;
    float radZ = rotation.z * M_PI / 180.0f;

    // Set up the model matrix
    model.setIdentity();
    translate(model, position);
    rotateX(model, radX);
    rotateY(model, radY);
    rotateZ(model, radZ);

    // Enable 3D rendering states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUseProgram(shape3DShaderProgram);
    glUniform3f(shape3DColorUniform, currentColor.r, currentColor.g, currentColor.b);
    glUniformMatrix4fv(shape3DProjectionUniform, 1, GL_FALSE, projection3D.m);
    glUniformMatrix4fv(shape3DViewUniform, 1, GL_FALSE, view3D.m);
    glUniformMatrix4fv(shape3DModelUniform, 1, GL_FALSE, model.m);

    // Bind and draw the cube
    glBindVertexArray(shape3DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape3DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape3DEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

int l_drawCube(lua_State* L) {
    Vector3* position = checkVector3(L, 1);
    Vector3* size = checkVector3(L, 2);
    Vector3* rotation = checkVector3(L, 3);

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    framework->drawCube(*position, *size, *rotation);
    return 0;
}

void VoltaFramework::cleanupOpenGL2D() {
    glDeleteVertexArrays(1, &shape2DVAO);
    glDeleteBuffers(1, &shape2DVBO);
    glDeleteBuffers(1, &shape2DEBO);
    glDeleteProgram(shape2DShaderProgram);
}

void VoltaFramework::cleanupOpenGL3D() {
    glDeleteVertexArrays(1, &shape3DVAO);
    glDeleteBuffers(1, &shape3DVBO);
    glDeleteBuffers(1, &shape3DEBO);
    glDeleteProgram(shape3DShaderProgram);
}

void VoltaFramework::cleanupOpenGL() {
    cleanupOpenGL2D();
    cleanupOpenGL3D();

    // Cleanup image rendering
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteProgram(imageShaderProgram);

    // Cleanup text rendering
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShaderProgram);

    // Cleanup particles
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);

    // Cleanup font textures
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
}