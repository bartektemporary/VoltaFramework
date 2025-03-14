#include "VoltaFramework.hpp"
#include "Vector2.hpp"
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstring>

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
    
    // Calculate corners in OpenGL coordinates
    float left, top, right, bottom;
    framework->windowToGLCoords(position->x, position->y, &left, &top);
    framework->windowToGLCoords(position->x + size->x, position->y + size->y, &right, &bottom);
    
    if (fill) {
        // Setup vertices for filled rectangle
        float vertices[] = {
            left, top,
            right, top,
            right, bottom,
            left, bottom
        };
        
        // Draw filled rectangle
        glUseProgram(framework->shaderProgram);
        glUniform3fv(framework->colorUniform, 1, framework->currentColor);
        glUniform1i(framework->useTextureUniform, 0); // No texture
        
        glBindVertexArray(framework->shapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } else {
        // Setup vertices for rectangle outline
        float vertices[] = {
            left, top,
            right, top,
            right, bottom,
            left, bottom
        };
        
        // Draw rectangle outline
        glUseProgram(framework->shaderProgram);
        glUniform3fv(framework->colorUniform, 1, framework->currentColor);
        glUniform1i(framework->useTextureUniform, 0); // No texture
        
        glBindVertexArray(framework->shapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    
    glBindVertexArray(0);
    
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
    
    // Convert center to OpenGL coordinates
    float centerX, centerY;
    framework->windowToGLCoords(center->x, center->y, &centerX, &centerY);
    
    // Calculate radius in OpenGL coordinates
    float radiusX = r / (framework->getWidth() / 2.0f);
    float radiusY = r / (framework->getHeight() / 2.0f);
    
    // Generate vertex data for the circle
    const int segments = 35;
    std::vector<float> vertices;
    
    if (fill) {
        // Add center vertex for triangle fan
        vertices.push_back(centerX);
        vertices.push_back(centerY);
    }
    
    // Generate vertices around the circle
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = centerX + radiusX * cosf(theta);
        float y = centerY + radiusY * sinf(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }
    
    // Draw the circle
    glUseProgram(framework->shaderProgram);
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 0); // No texture
    
    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    if (fill) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
    }
    
    glBindVertexArray(0);
    
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
    
    // Convert to OpenGL coordinates
    float startX, startY, endX, endY;
    framework->windowToGLCoords(start->x, start->y, &startX, &startY);
    framework->windowToGLCoords(end->x, end->y, &endX, &endY);
    
    // Calculate the direction of the line
    float dx = endX - startX;
    float dy = endY - startY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length == 0) {
        // If the line has no length, there's nothing to draw
        return 0;
    }
    
    // Normalize the direction vector
    float nx = dx / length;
    float ny = dy / length;
    
    // Calculate the perpendicular vector (rotate 90 degrees)
    float perpX = -ny;
    float perpY = nx;
    
    // Calculate the half-width in NDC space
    float halfWidth = lineWidth / framework->getWidth();
    
    // Compute the four corners of the quad
    float halfWidthPerpX = perpX * halfWidth;
    float halfWidthPerpY = perpY * halfWidth;
    
    // Define the four corners of the quad
    float vertices[] = {
        startX - halfWidthPerpX, startY - halfWidthPerpY,
        startX + halfWidthPerpX, startY + halfWidthPerpY,
        endX + halfWidthPerpX, endY + halfWidthPerpY,
        endX - halfWidthPerpX, endY - halfWidthPerpY
    };
    
    // Draw the quad
    glUseProgram(framework->shaderProgram);
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 0); // No texture
    
    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBindVertexArray(0);
    
    return 0;
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

    // Convert to OpenGL coordinates
    float left, top, right, bottom;
    framework->windowToGLCoords(position->x, position->y, &left, &top);
    framework->windowToGLCoords(position->x + size->x, position->y + size->y, &right, &bottom);
    
    // Load the texture
    std::string fullPath {std::string("assets/") + filename};
    GLuint textureID {framework->loadTexture(fullPath)};
    if (textureID == 0) {
        return 0;
    }
    
    // Set up texture coordinates
    float vertices[] = {
        // Positions    // Texture Coords
        left, top,      0.0f, 1.0f,
        right, top,     1.0f, 1.0f,
        right, bottom,  1.0f, 0.0f,
        left, bottom,   0.0f, 0.0f
    };
    
    // Draw the textured quad
    glUseProgram(framework->shaderProgram);
    glUniform3fv(framework->colorUniform, 1, framework->currentColor);
    glUniform1i(framework->useTextureUniform, 1); // Use texture
    glUniform1i(framework->textureUniform, 0); // Texture unit 0
    
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
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Apply current filter mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    FreeImage_Unload(bitmap32);

    textureCache[filename] = textureID;
    return textureID;
}


// Shader source code
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

// Shader compilation utility function
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check compilation status
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

// Shader program creation utility function
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check linking status
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    
    // Clean up shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

// Modern OpenGL initialization function for VoltaFramework
void VoltaFramework::initOpenGL() {
    // Create shader program
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    
    // Get uniform locations
    colorUniform = glGetUniformLocation(shaderProgram, "uColor");
    useTextureUniform = glGetUniformLocation(shaderProgram, "uUseTexture");
    textureUniform = glGetUniformLocation(shaderProgram, "uTexture");
    
    // Create VAO and VBO for shape rendering
    glGenVertexArrays(1, &shapeVAO);
    glGenBuffers(1, &shapeVBO);
    glGenBuffers(1, &shapeEBO);
    
    // Create VAO and VBO for texture rendering
    glGenVertexArrays(1, &textureVAO);
    glGenBuffers(1, &textureVBO);
    
    // Set up rectangle vertices and UV coordinates
    float rectVertices[] = {
        // Positions    // Texture Coords
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f, -1.0f,   0.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    // Set up VBO and EBO for texture rendering
    glBindVertexArray(textureVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Set up VBO for shape rendering
    glBindVertexArray(shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);
    
    // Position attribute only for shapes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind VAO
    glBindVertexArray(0);
    
    // Set default color
    currentColor[0] = 1.0f;
    currentColor[1] = 1.0f;
    currentColor[2] = 1.0f;
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VoltaFramework::cleanupOpenGL() {
    glDeleteVertexArrays(1, &shapeVAO);
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteBuffers(1, &shapeVBO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteBuffers(1, &shapeEBO);
    glDeleteProgram(shaderProgram);
}