#ifndef VOLTA_FRAMEWORK_H
#define VOLTA_FRAMEWORK_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "Json.hpp"
#include "Buffer.hpp"
#include "Tweens.hpp"
#include "Color.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lua.hpp>
#include "miniaudio.h"
#include <FreeImage.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <sqlite3.h>

struct Vector2 {
    float x;
    float y;
    Vector2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}
};

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    float size;
    GLuint texture = 0;
};

class VoltaFramework;

enum class EmitterShape {
    Circle,
    Rectangle,
    Cone,
    Line
};

class ParticleEmitter {
public:
    ParticleEmitter(Vector2 position, float particleLife, float speed, float spread, Vector2 direction = {1.0f, 0.0f}, 
        EmitterShape shape = EmitterShape::Cone, float width = 100.0f, float height = 100.0f, GLuint texture = 0);
    void emit(int count);
    void update(float dt);
    void render(VoltaFramework* framework);
    
    void setPosition(Vector2 newPos) { position = newPos; }
    Vector2 getPosition() const { return position; }
    void setShape(EmitterShape newShape) { shape = newShape; }
    EmitterShape getShape() const { return shape; }
    void setSize(float newWidth, float newHeight) { width = newWidth; height = newHeight; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    
    float particleLife;
    float speed;
    float spread;
    Vector2 direction;

    void setParticleTexture(GLuint texture) { particleTexture = texture; }
    GLuint getParticleTexture() const { return particleTexture; }
    
private:
    Vector2 position;
    std::vector<Particle> particles;
    EmitterShape shape;
    float width;
    float height;
    GLuint particleTexture;
};

class VoltaFramework {
public:
    VoltaFramework();
    ~VoltaFramework();
    void run();
    double startTime;
    GLFWwindow* getWindow() const { return window; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    void setWidth(int newWidth) { width = newWidth; }
    void setHeight(int newHeight) { height = newHeight; }

    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }

    int getState() const { return state; }
    void setState(int newState) { state = newState; }

    GLuint loadTexture(const std::string& filename);
    void setFilterMode(GLenum newMode) { filterMode = newMode; }

    ma_sound* loadAudio(const std::string& filename);
    void setGlobalVolume(float volume);
    float getGlobalVolume() const { return globalVolume; }

    void registerKeyPressCallback(const std::string& key, int ref);
    void registerMouseButtonPressCallback(const std::string& button, int ref);

    void jsonToLua(lua_State* L, const json::Value& value);
    json::Value* luaToJson(lua_State* L, int index);

    std::unordered_map<Buffer*, std::unique_ptr<Buffer>> bufferCache;

    struct Character {
        GLuint textureID;
        Vector2 size;
        Vector2 bearing;
        unsigned int advance;
    };

    FT_Library ftLibrary;
    FT_Face ftFace;
    std::unordered_map<std::string, FT_Face> fontCache;
    std::unordered_map<char, struct Character> characters;
    GLuint textVAO, textVBO;

    void loadFont(const std::string& fontPath, unsigned int fontSize);
    void setFont(const std::string& fontPath);
    void drawText(const std::string& text, float x, float y, float scale = 1.0f);


    // Shader programs for different rendering tasks
    GLuint shapeShaderProgram;  // For shapes (rectangles, circles, lines)
    GLuint imageShaderProgram;  // For images (like tree.png)
    GLuint textShaderProgram;   // For text rendering
    // Uniforms for shape shader
    GLint shapeColorUniform;
    // Uniforms for image shader
    GLint imageColorUniform;
    GLint imageTextureUniform;
    // Uniforms for text shader
    GLint textColorUniform;
    GLint textTextureUniform;
    
    GLuint shapeVAO, shapeVBO, shapeEBO;
    GLuint textureVAO, textureVBO;
    GLint colorUniform, useTextureUniform, textureUniform;
    float currentColor[3];

    void initOpenGL();
    void cleanupOpenGL();

    void windowToGLCoords(float winX, float winY, float* glX, float* glY);

    void registerCustomEventCallback(const std::string& eventName, int ref);
    void triggerCustomEvent(const std::string& eventName, lua_State* L, int nargs);

    void renderParticles(float dt);
    std::vector<ParticleEmitter> particleEmitters;
    GLuint particleVAO, particleVBO;

    bool isGamepadConnected(int gamepadId) const;
    bool isGamepadButtonDown(int gamepadId, int button) const;

    void registerGamepadConnectedCallback(int ref);
    void registerGamepadDisconnectedCallback(int ref);
    void registerGamepadButtonPressedCallback(int button, int ref);

    bool createCustomShader(const std::string& shaderName, const std::string& vertexSource, const std::string& fragmentSource);
    bool createCustomShaderFromFiles(const std::string& shaderName, const std::string& vertexFile, const std::string& fragmentFile);
    void setShader(const std::string& shaderName);
    void clearCustomShader(const std::string& shaderName);
    void clearAllCustomShaders();
    bool hasShader(const std::string& shaderName) const;
    void useCustomShader(bool use);

    struct ShaderProgram {
        GLuint program;
        GLint colorUniform;
        GLint useTextureUniform;
        GLint textureUniform;
    };

    std::unordered_map<std::string, ShaderProgram> customShaders;
    std::string currentShaderName;
    bool usingCustomShader;

    GLuint compileShader(GLenum type, const char* source);
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
    void setShaderUniform(const std::string& name, float value);
    void setShaderUniform(const std::string& name, const Vector2& value);

    std::unordered_map<std::string, sqlite3*> databaseCache;

private:
    lua_State* L;
    GLFWwindow* window;
    int width;
    int height;
    int x;
    int y;
    int state;

    std::unordered_map<std::string, GLuint> textureCache;
    std::unordered_map<std::string, ma_sound> audioCache;
    GLenum filterMode;
    ma_engine engine;
    float globalVolume;

    std::unordered_map<int, std::vector<int>> keyPressCallbackRefs;
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    std::unordered_map<int, std::vector<int>> mouseButtonCallbackRefs;
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    void loadLuaScript(const std::string& filename);
    void update(float dt);
    void registerLuaAPI();

    static void windowSizeCallback(GLFWwindow* window, int newWidth, int newHeight);
    static void windowPosCallback(GLFWwindow* window, int xpos, int ypos);
    static void windowMaximizeCallback(GLFWwindow* window, int maximized);

    std::unordered_map<std::string, std::vector<int>> customEventCallbackRefs;

    std::unordered_map<int, bool> gamepadStates;
    std::vector<int> gamepadConnectedCallbackRefs;
    std::vector<int> gamepadDisconnectedCallbackRefs;
    std::unordered_map<int, std::vector<int>> gamepadButtonPressedCallbackRefs;

    static void joystickCallback(int jid, int event);
};

extern VoltaFramework* g_frameworkInstance;
VoltaFramework* getFramework(lua_State* L);

int l_window_setTitle(lua_State* L);
int l_window_getTitle(lua_State* L);
int l_window_setSize(lua_State* L);
int l_window_getSize(lua_State* L);
int l_window_setPosition(lua_State* L);
int l_window_getPosition(lua_State* L);
int l_window_setResizable(lua_State* L);
int l_window_setFullscreen(lua_State* L);
int l_window_setState(lua_State* L);
int l_window_getState(lua_State* L);
int l_window_setIcon(lua_State* L);
int l_window_setVsync(lua_State* L);

int l_rectangle(lua_State* L);
int l_circle(lua_State* L);
int l_drawLine(lua_State* L);
int l_setColor(lua_State* L);
int l_drawImage(lua_State* L);
int l_loadFont(lua_State* L);
int l_setFont(lua_State* L);
int l_drawText(lua_State* L);
int l_setFilter(lua_State* L);
int l_setCustomShader(lua_State* L);
int l_setShader(lua_State* L);
int l_useCustomShader(lua_State* L);
int l_clearCustomShader(lua_State* L);
int l_setCustomShaderUniform(lua_State* L);

int l_isKeyDown(lua_State* L);
int l_input_keyPressed(lua_State* L);
int l_input_getPressedKeys(lua_State* L);
int l_input_getMousePosition(lua_State* L);
int l_input_isMouseButtonDown(lua_State* L);
int l_input_mouseButtonPressed(lua_State* L);
int l_input_getPressedMouseButtons(lua_State* L);
int l_input_isGamepadConnected(lua_State* L);
int l_input_isGamepadButtonDown(lua_State* L);
int l_input_gamepadConnected(lua_State* L);
int l_input_gamepadDisconnected(lua_State* L);
int l_input_getGamepadButtonPressed(lua_State* L);

int l_audio_loadAudio(lua_State* L);
int l_audio_play(lua_State* L);
int l_audio_stop(lua_State* L);
int l_audio_setVolume(lua_State* L);
int l_audio_getVolume(lua_State* L);
int l_audio_setLooped(lua_State* L);
int l_audio_setGlobalVolume(lua_State* L);
int l_audio_getGlobalVolume(lua_State* L);

int l_filesystem_exists(lua_State* L);
int l_filesystem_isDirectory(lua_State* L);
int l_filesystem_createDirectory(lua_State* L);
int l_filesystem_remove(lua_State* L);
int l_filesystem_listDir(lua_State* L);
int l_filesystem_getWorkingDir(lua_State* L);
int l_filesystem_setWorkingDir(lua_State* L);
int l_filesystem_getFileSize(lua_State* L);
int l_filesystem_isRegularFile(lua_State* L);
int l_filesystem_isCharacterFile(lua_State* L);
int l_filesystem_isFifo(lua_State* L);
int l_filesystem_isSocket(lua_State* L);
int l_filesystem_canonical(lua_State* L);
int l_filesystem_relative(lua_State* L);
int l_filesystem_copy(lua_State* L);
int l_filesystem_rename(lua_State* L);
int l_filesystem_lastWriteTime(lua_State* L);
int l_filesystem_isEmpty(lua_State* L);
int l_filesystem_absolute(lua_State* L);
int l_filesystem_parentPath(lua_State* L);
int l_filesystem_filename(lua_State* L);
int l_filesystem_stem(lua_State* L);
int l_filesystem_extension(lua_State* L);
int l_filesystem_tempDirectory(lua_State* L);
int l_filesystem_equivalent(lua_State* L);
int l_filesystem_getUserDir(lua_State* L);

int l_json_decode(lua_State* L);
int l_json_encode(lua_State* L);

int l_sqlite_open(lua_State* L);
int l_sqlite_close(lua_State* L);
int l_sqlite_exec(lua_State* L);
int l_sqlite_prepare(lua_State* L);
int l_sqlite_step(lua_State* L);
int l_sqlite_finalize(lua_State* L);
int l_sqlite_bind(lua_State* L);
int l_sqlite_column(lua_State* L);

int l_buffer_alloc(lua_State* L);
int l_buffer_writeUInt8(lua_State* L);
int l_buffer_readUInt8(lua_State* L);
int l_buffer_writeInt8(lua_State* L);
int l_buffer_readInt8(lua_State* L);
int l_buffer_writeUInt16(lua_State* L);
int l_buffer_readUInt16(lua_State* L);
int l_buffer_writeInt16(lua_State* L);
int l_buffer_readInt16(lua_State* L);
int l_buffer_writeUInt32(lua_State* L);
int l_buffer_readUInt32(lua_State* L);
int l_buffer_writeInt32(lua_State* L);
int l_buffer_readInt32(lua_State* L);
int l_buffer_writeUInt64(lua_State* L);
int l_buffer_readUInt64(lua_State* L);
int l_buffer_writeInt64(lua_State* L);
int l_buffer_readInt64(lua_State* L);
int l_buffer_size(lua_State* L);

int l_vector2_new(lua_State* L);
int l_vector2_add(lua_State* L);
int l_vector2_subtract(lua_State* L);
int l_vector2_multiply(lua_State* L);
int l_vector2_divide(lua_State* L);
int l_vector2_magnitude(lua_State* L);
int l_vector2_normalize(lua_State* L);
int l_vector2_dot(lua_State* L);
int l_vector2_lerp(lua_State* L);
int l_vector2_distance(lua_State* L);
int l_vector2_angle(lua_State* L);
int l_vector2_tween(lua_State* L);
int l_vector2_tostring(lua_State* L);

int l_particleEmitter_new(lua_State* L);
int l_particleEmitter_emit(lua_State* L);
int l_particleEmitter_render(lua_State* L);
int l_particleEmitter_setShape(lua_State* L);
int l_particleEmitter_getShape(lua_State* L);
int l_particleEmitter_setSize(lua_State* L);
int l_particleEmitter_getSize(lua_State* L);
int l_particleEmitter_setDirection(lua_State* L);
int l_particleEmitter_getDirection(lua_State* L);
int l_particleEmitter_setLifetime(lua_State* L);
int l_particleEmitter_setSpeed(lua_State* L);
int l_particleEmitter_setSpread(lua_State* L);
int l_particleEmitter_getLifetime(lua_State* L);
int l_particleEmitter_getSpeed(lua_State* L);
int l_particleEmitter_getSpread(lua_State* L);
int l_particleEmitter_setPosition(lua_State* L);
int l_particleEmitter_getPosition(lua_State* L);
int l_particleEmitter_setTexture(lua_State* L);

int l_onCustomEvent(lua_State* L);
int l_triggerCustomEvent(lua_State* L);

int l_math_clamp(lua_State* L);
int l_math_round(lua_State* L);
int l_math_lerp(lua_State* L);
int l_math_tween(lua_State* L);
int l_math_noise1d(lua_State* L);
int l_math_noise2d(lua_State* L);
int l_math_noise3d(lua_State* L);

int l_table_shallowCopy(lua_State* L);

int l_getRunningTime(lua_State* L);

#endif // VOLTA_FRAMEWORK_H