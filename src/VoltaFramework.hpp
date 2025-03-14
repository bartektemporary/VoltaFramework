#ifndef VOLTA_FRAMEWORK_H
#define VOLTA_FRAMEWORK_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>

#include "json.hpp"
#include "Buffer.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lua.hpp>
#include "miniaudio.h"
#include <FreeImage.h>

struct Vector2 {
    float x;
    float y;
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

    GLuint shaderProgram;
    GLuint shapeVAO, shapeVBO, shapeEBO;
    GLuint textureVAO, textureVBO;
    GLint colorUniform, useTextureUniform, textureUniform;
    float currentColor[3];

    void initOpenGL();
    void cleanupOpenGL();

    void windowToGLCoords(float winX, float winY, float* glX, float* glY);

    void registerCustomEventCallback(const std::string& eventName, int ref);
    void triggerCustomEvent(const std::string& eventName, lua_State* L, int nargs);

private:
    lua_State* L;
    GLFWwindow* window;
    int width;
    int height;
    int x;
    int y;
    int state;

    std::unordered_map<std::string, GLuint> textureCache {};
    std::unordered_map<std::string, ma_sound> audioCache {};
    GLenum filterMode;
    ma_engine engine;
    float globalVolume;

    std::unordered_map<int, std::vector<int>> keyPressCallbackRefs {};
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    std::unordered_map<int, std::vector<int>> mouseButtonCallbackRefs {};
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    void loadLuaScript(const std::string& filename);
    void update(float dt);
    void registerLuaAPI();

    static void windowSizeCallback(GLFWwindow* window, int newWidth, int newHeight);
    static void windowPosCallback(GLFWwindow* window, int xpos, int ypos);
    static void windowMaximizeCallback(GLFWwindow* window, int maximized);

    std::unordered_map<std::string, std::vector<int>> customEventCallbackRefs {};
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
int l_setFilter(lua_State* L);

int l_isKeyDown(lua_State* L);
int l_input_keyPressed(lua_State* L);
int l_input_isMouseButtonDown(lua_State* L);
int l_input_getMousePosition(lua_State* L);
int l_input_mouseButtonPressed(lua_State* L);

int l_audio_loadAudio(lua_State* L);
int l_audio_play(lua_State* L);
int l_audio_stop(lua_State* L);
int l_audio_setVolume(lua_State* L);
int l_audio_getVolume(lua_State* L);
int l_audio_setLooped(lua_State* L);
int l_audio_setGlobalVolume(lua_State* L);
int l_audio_getGlobalVolume(lua_State* L);

int l_json_decode(lua_State* L);
int l_json_encode(lua_State* L);

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

Vector2* checkVector2(lua_State* L, int index);

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