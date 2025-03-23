#include "VoltaFramework.hpp"
#include "Vector2.hpp"
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
    float centerX, centerY;
    windowToGLCoords(position.x + size.x / 2.0f, position.y + size.y / 2.0f, &centerX, &centerY);
    
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
        windowToGLCoords(position.x + halfWidth + rotatedX, position.y + halfHeight + rotatedY, &glX, &glY);
        rotatedVertices.push_back(glX);
        rotatedVertices.push_back(glY);
    }
    
    float vertices[8];
    for (int i = 0; i < 8; i++) {
        vertices[i] = rotatedVertices[i];
    }
    
    glDisable(GL_DEPTH_TEST); // 2D rendering, no depth
    glUseProgram(shape2DShaderProgram);
    glUniform3fv(shape2DColorUniform, 1, currentColor);

    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

void VoltaFramework::drawCircle(bool fill, const Vector2& center, float radius) {
    float centerX, centerY;
    windowToGLCoords(center.x, center.y, &centerX, &centerY);
    
    float radiusX = radius / (width / 2.0f);
    float radiusY = radius / (height / 2.0f);
    
    const int segments = 35;
    std::vector<float> vertices;
    
    if (fill) {
        vertices.push_back(centerX);
        vertices.push_back(centerY);
    }
    
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = centerX + radiusX * cosf(theta);
        float y = centerY + radiusY * sinf(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }
    
    glDisable(GL_DEPTH_TEST); // 2D rendering
    glUseProgram(shape2DShaderProgram);
    glUniform3fv(shape2DColorUniform, 1, currentColor);
    
    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
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
    float startX, startY, endX, endY;
    windowToGLCoords(start.x, start.y, &startX, &startY);
    windowToGLCoords(end.x, end.y, &endX, &endY);
    
    float dx = endX - startX;
    float dy = endY - startY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length == 0) {
        return;
    }
    
    float nx = dx / length;
    float ny = dy / length;
    
    float perpX = -ny;
    float perpY = nx;
    
    // Adjust line width based on window dimensions
    float halfWidth = (lineWidth / 2.0f) / (width / 2.0f); // Normalize to OpenGL coordinates
    
    float halfWidthPerpX = perpX * halfWidth;
    float halfWidthPerpY = perpY * halfWidth;
    
    float vertices[] = {
        startX - halfWidthPerpX, startY - halfWidthPerpY,
        startX + halfWidthPerpX, startY + halfWidthPerpY,
        endX + halfWidthPerpX, endY + halfWidthPerpY,
        endX - halfWidthPerpX, endY - halfWidthPerpY
    };
    
    // Ensure 2D rendering state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(shape2DShaderProgram);
    glUniform3fv(shape2DColorUniform, 1, currentColor);
    
    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
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

    float centerX, centerY;
    framework->windowToGLCoords(position->x + size->x/2.0f, position->y + size->y/2.0f, &centerX, &centerY);
    
    float halfWidth = size->x / 2.0f;
    float halfHeight = size->y / 2.0f;
    
    float rad = -rotation * M_PI / 180.0f;
    float cosR = cosf(rad);
    float sinR = sinf(rad);
    
    // Precompute rotated corners
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
        framework->windowToGLCoords(position->x + halfWidth + rotatedX, position->y + halfHeight + rotatedY, &glX, &glY);
        rotatedVertices.push_back(glX);
        rotatedVertices.push_back(glY);
    }
    
    // Correct texture coordinates (0,0 to 1,1 across all vertices)
    float vertices[16] = {
        rotatedVertices[0], rotatedVertices[1], 0.0f, 1.0f,
        rotatedVertices[2], rotatedVertices[3], 1.0f, 1.0f,
        rotatedVertices[4], rotatedVertices[5], 1.0f, 0.0f,
        rotatedVertices[6], rotatedVertices[7], 0.0f, 0.0f
    };
    
    std::string fullPath = std::string("assets/") + filename;
    GLuint textureID = framework->loadTexture(fullPath);
    if (textureID == 0) {
        return 0;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (framework->usingCustomShader && !framework->currentShaderName.empty()) {
        auto& shader = framework->customShaders[framework->currentShaderName];
        glUseProgram(shader.program);
        if (shader.colorUniform != -1)
            glUniform3fv(shader.colorUniform, 1, framework->currentColor);
        if (shader.useTextureUniform != -1)
            glUniform1i(shader.useTextureUniform, 1);
        if (shader.textureUniform != -1)
            glUniform1i(shader.textureUniform, 0);
    } else {
        glUseProgram(framework->imageShaderProgram); // Use image shader
        glUniform3fv(framework->imageColorUniform, 1, framework->currentColor);
        glUniform1i(framework->imageTextureUniform, 0);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glBindVertexArray(framework->textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);

    return 0;
}

int l_setColor(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "l_setColor: Framework is null\n";
        return 0;
    }

    if (lua_isuserdata(L, 1)) {
        Color* color = checkColor(L, 1);
        framework->currentColor[0] = color->r;
        framework->currentColor[1] = color->g;
        framework->currentColor[2] = color->b;
    } else {
        lua_Number r = luaL_checknumber(L, 1);
        lua_Number g = luaL_checknumber(L, 2);
        lua_Number b = luaL_checknumber(L, 3);
        framework->currentColor[0] = static_cast<float>(r);
        framework->currentColor[1] = static_cast<float>(g);
        framework->currentColor[2] = static_cast<float>(b);
    }
    return 0;
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

bool VoltaFramework::createCustomShaderFromFiles(const std::string& shaderName, const std::string& vertexFile, const std::string& fragmentFile) {
    std::string vertexSource = loadFile(vertexFile, true);
    if (vertexSource.empty()) {
        std::cerr << "Failed to load vertex shader file: " << vertexFile << "\n";
        return false;
    }

    std::string fragmentSource = loadFile(fragmentFile, true);
    if (fragmentSource.empty()) {
        std::cerr << "Failed to load fragment shader file: " << fragmentFile << "\n";
        return false;
    }

    return createCustomShader(shaderName, vertexSource, fragmentSource);
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

void VoltaFramework::setShaderUniform(const std::string& name, float value) {
    if (usingCustomShader && !currentShaderName.empty()) {
        GLuint program = customShaders[currentShaderName].program;
        glUseProgram(program); // Ensure the shader is active
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
    if (!ftFace || characters.empty()) {
        std::cerr << "No font loaded for drawText\n";
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLenum blendStatus = glIsEnabled(GL_BLEND);

    glUseProgram(textShaderProgram); // Use text shader
    glUniform3fv(textColorUniform, 1, currentColor);
    glUniform1i(textTextureUniform, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float cursorX = x;
    for (char c : text) {
        if (c == '\n') {
            cursorX = x;
            y -= (characters['A'].size.y + 10) * scale;
            continue;
        }

        auto it = characters.find(c);
        if (it == characters.end()) {
            std::cerr << "Glyph not found for character: " << (int)c << " ('" << c << "')\n";
            continue;
        }

        Character& ch = it->second;

        float advancePixels = (float)ch.advance * scale;

        float xpos, ypos;
        windowToGLCoords(cursorX + ch.bearing.x * scale, y - (ch.size.y - ch.bearing.y) * scale, &xpos, &ypos);
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

        cursorX += advancePixels;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
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
    // 2D Shape Shader
    shape2DShaderProgram = createShaderProgram(shapeVertexShaderSource2D, shapeFragmentShaderSource);
    if (shape2DShaderProgram == 0) {
        std::cerr << "Failed to create 2D shape shader program\n";
        return;
    }
    shape2DColorUniform = glGetUniformLocation(shape2DShaderProgram, "uColor");

    // 2D VAO
    glGenVertexArrays(1, &shape2DVAO);
    glGenBuffers(1, &shape2DVBO);
    glGenBuffers(1, &shape2DEBO);
    glBindVertexArray(shape2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape2DVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape2DEBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Orthographic projection for 2D
    projection2D.setIdentity();
    float left = 0.0f, right = (float)width, bottom = 0.0f, top = (float)height;
    projection2D.m[0] = 2.0f / (right - left);
    projection2D.m[5] = 2.0f / (top - bottom);
    projection2D.m[12] = -(right + left) / (right - left);
    projection2D.m[13] = -(top + bottom) / (top - bottom);
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
    glUniform3fv(shape3DColorUniform, 1, currentColor);
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