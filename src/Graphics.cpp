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
    *glY = 1.0f - (winY / (height / 2.0f));
}

int l_rectangle(lua_State* L) {
    int isbool {lua_isboolean(L, 1)};
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill {lua_toboolean(L, 1) != 0};
    
    Vector2* position = checkVector2(L, 2);
    Vector2* size = checkVector2(L, 3);
    lua_Number rotation {luaL_optnumber(L, 4, 0.0)};

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_rectangle: Framework or window is null\n";
        return 0;
    }
    
    float centerX, centerY;
    framework->windowToGLCoords(position->x + size->x/2.0f, position->y + size->y/2.0f, &centerX, &centerY);
    
    float halfWidth = size->x / 2.0f;
    float halfHeight = size->y / 2.0f;
    
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
        framework->windowToGLCoords(position->x + halfWidth + rotatedX, position->y + halfHeight + rotatedY, &glX, &glY);
        rotatedVertices.push_back(glX);
        rotatedVertices.push_back(glY);
    }
    
    float vertices[8];
    for (int i = 0; i < 8; i++) {
        vertices[i] = rotatedVertices[i];
    }
    
    glUseProgram(framework->shaderProgram); // Explicitly bind shader
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 0);

    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Ensure attribute is enabled
    
    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    
    glBindVertexArray(0); // Unbind VAO but leave shader active
    return 0;
}

int l_circle(lua_State* L) {
    int isbool {lua_isboolean(L, 1)};
    if (!isbool) {
        luaL_argerror(L, 1, "boolean expected");
    }
    bool fill {lua_toboolean(L, 1) != 0};
    
    Vector2* center = checkVector2(L, 2);
    lua_Number r {luaL_checknumber(L, 3)};

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_circle: Framework or window is null\n";
        return 0;
    }
    
    float centerX, centerY;
    framework->windowToGLCoords(center->x, center->y, &centerX, &centerY);
    
    float radiusX = r / (framework->getWidth() / 2.0f);
    float radiusY = r / (framework->getHeight() / 2.0f);
    
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
    
    glUseProgram(framework->shaderProgram); // Explicitly bind shader
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 0);
    
    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Ensure attribute is enabled
    
    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
    }
    
    glBindVertexArray(0);
    return 0;
}

int l_drawLine(lua_State* L) {
    Vector2* start = checkVector2(L, 1);
    Vector2* end = checkVector2(L, 2);
    lua_Number lineWidth {luaL_optnumber(L, 3, 1.0)};

    if (lineWidth <= 0) {
        luaL_argerror(L, 3, "line width must be positive");
    }

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawLine: Framework or window is null\n";
        return 0;
    }
    
    float startX, startY, endX, endY;
    framework->windowToGLCoords(start->x, start->y, &startX, &startY);
    framework->windowToGLCoords(end->x, end->y, &endX, &endY);
    
    float dx = endX - startX;
    float dy = endY - startY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length == 0) {
        return 0;
    }
    
    float nx = dx / length;
    float ny = dy / length;
    
    float perpX = -ny;
    float perpY = nx;
    
    float halfWidth = lineWidth / framework->getWidth();
    
    float halfWidthPerpX = perpX * halfWidth;
    float halfWidthPerpY = perpY * halfWidth;
    
    float vertices[] = {
        startX - halfWidthPerpX, startY - halfWidthPerpY,
        startX + halfWidthPerpX, startY + halfWidthPerpY,
        endX + halfWidthPerpX, endY + halfWidthPerpY,
        endX - halfWidthPerpX, endY - halfWidthPerpY
    };
    
    glUseProgram(framework->shaderProgram); // Explicitly bind shader
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 0);
    
    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Ensure attribute is enabled
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBindVertexArray(0);
    return 0;
}

int l_drawImage(lua_State* L) {
    const char* filename {luaL_checkstring(L, 1)};
    Vector2* position = checkVector2(L, 2);
    Vector2* size = checkVector2(L, 3);
    lua_Number rotation {luaL_optnumber(L, 4, 0.0)}; // Optional rotation in degrees

    VoltaFramework* framework {getFramework(L)};
    if (!framework || !framework->getWindow()) {
        std::cerr << "l_drawImage: Framework or window is null\n";
        return 0;
    }

    // Convert center position to GL coordinates
    float centerX, centerY;
    framework->windowToGLCoords(position->x + size->x/2.0f, position->y + size->y/2.0f, &centerX, &centerY);
    
    // Calculate half sizes in window coordinates
    float halfWidth = size->x / 2.0f;
    float halfHeight = size->y / 2.0f;
    
    // Convert rotation to radians (negative for OpenGL Y-axis)
    float rad = -rotation * M_PI / 180.0f;
    float cosR = cosf(rad);
    float sinR = sinf(rad);
    
    // Define corners in window coordinate space relative to center
    std::vector<float> rotatedVertices;
    float corners[8] = {
        -halfWidth, halfHeight,   // top-left
        halfWidth, halfHeight,    // top-right
        halfWidth, -halfHeight,   // bottom-right
        -halfWidth, -halfHeight   // bottom-left
    };
    
    for (int i = 0; i < 4; i++) {
        float x = corners[i * 2];
        float y = corners[i * 2 + 1];
        // Rotate around origin (0,0) in window space
        float rotatedX = x * cosR - y * sinR;
        float rotatedY = x * sinR + y * cosR;
        // Convert rotated coordinates to GL space
        float glX, glY;
        framework->windowToGLCoords(position->x + halfWidth + rotatedX, position->y + halfHeight + rotatedY, &glX, &glY);
        rotatedVertices.push_back(glX);
        rotatedVertices.push_back(glY);
    }
    
    // Adjust texture coordinates to ensure upright image at 0° rotation
    // OpenGL expects (0,0) at bottom-left, (1,1) at top-right
    float vertices[16]; // x, y, u, v for 4 vertices
    for (int i = 0; i < 4; i++) {
        vertices[i * 4] = rotatedVertices[i * 2];     // x
        vertices[i * 4 + 1] = rotatedVertices[i * 2 + 1]; // y
        // Fix texture coordinates to match vertex order
        if (i == 0) { // bottom-left
            vertices[i * 4 + 2] = 0.0f; // u
            vertices[i * 4 + 3] = 0.0f; // v
        } else if (i == 1) { // bottom-right
            vertices[i * 4 + 2] = 1.0f; // u
            vertices[i * 4 + 3] = 0.0f; // v
        } else if (i == 2) { // top-right
            vertices[i * 4 + 2] = 1.0f; // u
            vertices[i * 4 + 3] = 1.0f; // v
        } else if (i == 3) { // top-left
            vertices[i * 4 + 2] = 0.0f; // u
            vertices[i * 4 + 3] = 1.0f; // v
        }
    }
    
    std::string fullPath {std::string("assets/") + filename};
    GLuint textureID {framework->loadTexture(fullPath)};
    if (textureID == 0) {
        return 0;
    }
    
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
        glUseProgram(framework->shaderProgram);
        glUniform3fv(framework->colorUniform, 1, framework->currentColor);
        glUniform1i(framework->useTextureUniform, 1);
        glUniform1i(framework->textureUniform, 0);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glBindVertexArray(framework->textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    return 0;
}

int l_setColor(lua_State* L) {
    lua_Number r {luaL_checknumber(L, 1)};
    lua_Number g {luaL_checknumber(L, 2)};
    lua_Number b {luaL_checknumber(L, 3)};
    
    VoltaFramework* framework {getFramework(L)};
    if (!framework) {
        std::cerr << "l_setColor: Framework is null\n";
        return 0;
    }
    
    framework->currentColor[0] = static_cast<float>(r);
    framework->currentColor[1] = static_cast<float>(g);
    framework->currentColor[2] = static_cast<float>(b);
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

GLuint VoltaFramework::loadTexture(const std::string& filename) {
    std::string fullPath;
    
    if (strchr(filename.c_str(), '/') || strchr(filename.c_str(), '\\')) {
        fullPath = filename;
    } else {
        fullPath = "assets/" + filename;
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
    std::string vertexPath = "assets/" + vertexFile;
    std::ifstream vertexStream(vertexPath);
    if (!vertexStream.is_open()) {
        std::cerr << "Failed to open vertex shader file: " << vertexPath << "\n";
        return false;
    }
    std::stringstream vertexBuffer;
    vertexBuffer << vertexStream.rdbuf();
    std::string vertexSource = vertexBuffer.str();
    vertexStream.close();

    std::string fragmentPath = "assets/" + fragmentFile;
    std::ifstream fragmentStream(fragmentPath);
    if (!fragmentStream.is_open()) {
        std::cerr << "Failed to open fragment shader file: " << fragmentPath << "\n";
        return false;
    }
    std::stringstream fragmentBuffer;
    fragmentBuffer << fragmentStream.rdbuf();
    std::string fragmentSource = fragmentBuffer.str();
    fragmentStream.close();

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

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    
    uniform vec3 uColor;
    uniform bool uUseTexture;
    uniform sampler2D uTexture;
    
    void main() {
        if (uUseTexture) {
            FragColor = texture(uTexture, TexCoord) * vec4(uColor, 1.0);
        } else {
            FragColor = vec4(uColor, 1.0);
        }
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

void VoltaFramework::initOpenGL() {
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    
    colorUniform = glGetUniformLocation(shaderProgram, "uColor");
    useTextureUniform = glGetUniformLocation(shaderProgram, "uUseTexture");
    textureUniform = glGetUniformLocation(shaderProgram, "uTexture");
    
    // Shape VAO (for rectangles, circles, etc.)
    glGenVertexArrays(1, &shapeVAO);
    glGenBuffers(1, &shapeVBO);
    glGenBuffers(1, &shapeEBO);
    
    // Texture VAO (for images)
    glGenVertexArrays(1, &textureVAO);
    glGenBuffers(1, &textureVBO);
    
    float rectVertices[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f, -1.0f,   0.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Particle VAO (for particles)
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    currentColor[0] = 1.0f;
    currentColor[1] = 1.0f;
    currentColor[2] = 1.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VoltaFramework::cleanupOpenGL() {
    glDeleteVertexArrays(1, &shapeVAO);
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &shapeVBO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteBuffers(1, &particleVBO);
    glDeleteBuffers(1, &shapeEBO);
    glDeleteProgram(shaderProgram);
    clearAllCustomShaders();
}