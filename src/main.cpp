#include "VoltaFramework.hpp"
#include <cstring>

void VoltaFramework::registerLuaAPI() {
    // Register Vector2 metatable
    luaL_newmetatable(L, "Vector2");

    // Set arithmetic metamethods
    lua_pushcfunction(L, l_vector2_add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, l_vector2_subtract);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, l_vector2_multiply);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, l_vector2_divide);
    lua_setfield(L, -2, "__div");
    lua_pushcfunction(L, l_vector2_tostring);
    lua_setfield(L, -2, "__tostring");

    // Create methods table
    lua_newtable(L);
    lua_pushcfunction(L, l_vector2_add);
    lua_setfield(L, -2, "add");
    lua_pushcfunction(L, l_vector2_subtract);
    lua_setfield(L, -2, "subtract");
    lua_pushcfunction(L, l_vector2_multiply);
    lua_setfield(L, -2, "multiply");
    lua_pushcfunction(L, l_vector2_divide);
    lua_setfield(L, -2, "divide");
    lua_pushcfunction(L, l_vector2_magnitude);
    lua_setfield(L, -2, "magnitude");
    lua_pushcfunction(L, l_vector2_normalize);
    lua_setfield(L, -2, "normalize");
    lua_pushcfunction(L, l_vector2_dot);
    lua_setfield(L, -2, "dot");
    lua_pushcfunction(L, l_vector2_lerp);
    lua_setfield(L, -2, "lerp");
    lua_pushcfunction(L, l_vector2_distance);
    lua_setfield(L, -2, "distance");
    lua_pushcfunction(L, l_vector2_angle);
    lua_setfield(L, -2, "angle");
    lua_pushcfunction(L, l_vector2_tween);
    lua_setfield(L, -2, "tween");

    // Store methods table in the registry
    // Store methods table in the registry
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "Vector2Methods");

    // Set __index on the Vector2 metatable
    lua_pushcfunction(L, [](lua_State* L) {
        Vector2* vec = static_cast<Vector2*>(luaL_checkudata(L, 1, "Vector2"));
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "x") == 0) {
            lua_pushnumber(L, vec->x);
            return 1;
        }
        if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, vec->y);
            return 1;
        }

        lua_getfield(L, LUA_REGISTRYINDEX, "Vector2Methods");
        lua_getfield(L, -1, key);
        lua_remove(L, -2); // Remove the methods table, leaving the value
        return 1;
    });
    lua_setfield(L, -3, "__index");

    // Set __newindex on the Vector2 metatable to make fields read-only
    lua_pushcfunction(L, [](lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        luaL_error(L, "Cannot modify Vector2: field '%s' is read-only", key);
        return 0;
    });
    lua_setfield(L, -3, "__newindex");

    // Clean up: pop the methods table and Vector2 metatable
    lua_pop(L, 1); // Pop the methods table
    lua_pop(L, 1); // Pop the Vector2 metatable

    // Create the global 'volta' table
    lua_newtable(L);

    // Register volta.window
    lua_newtable(L);
    lua_pushcfunction(L, l_window_getTitle);
    lua_setfield(L, -2, "getTitle");
    lua_pushcfunction(L, l_window_setTitle);
    lua_setfield(L, -2, "setTitle");
    lua_pushcfunction(L, l_window_setSize);
    lua_setfield(L, -2, "setSize");
    lua_pushcfunction(L, l_window_getSize);
    lua_setfield(L, -2, "getSize");
    lua_pushcfunction(L, l_window_setPosition);
    lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, l_window_getPosition);
    lua_setfield(L, -2, "getPosition");
    lua_pushcfunction(L, l_window_setResizable);
    lua_setfield(L, -2, "setResizable");
    lua_pushcfunction(L, l_window_setFullscreen);
    lua_setfield(L, -2, "setFullscreen");
    lua_pushcfunction(L, l_window_setState);
    lua_setfield(L, -2, "setState");
    lua_pushcfunction(L, l_window_getState);
    lua_setfield(L, -2, "getState");
    lua_pushcfunction(L, l_window_setIcon);
    lua_setfield(L, -2, "setIcon");
    lua_pushcfunction(L, l_window_setVsync);
    lua_setfield(L, -2, "setVsync");
    lua_setfield(L, -2, "window");

    // Register volta.graphics
    lua_newtable(L);
    lua_pushcfunction(L, l_rectangle);
    lua_setfield(L, -2, "rectangle");
    lua_pushcfunction(L, l_circle);
    lua_setfield(L, -2, "circle");
    lua_pushcfunction(L, l_drawLine);
    lua_setfield(L, -2, "drawLine");
    lua_pushcfunction(L, l_setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, l_drawImage);
    lua_setfield(L, -2, "drawImage");
    lua_pushcfunction(L, l_setFilter);
    lua_setfield(L, -2, "setFilter");
    lua_setfield(L, -2, "graphics");

    // Register volta.input
    lua_newtable(L);
    lua_pushlightuserdata(L, window);
    lua_pushcclosure(L, l_isKeyDown, 1);
    lua_setfield(L, -2, "isKeyDown");
    lua_pushcfunction(L, l_input_keyPressed);
    lua_setfield(L, -2, "keyPressed");
    lua_pushcfunction(L, l_input_isMouseButtonDown);
    lua_setfield(L, -2, "isMouseButtonDown");
    lua_pushcfunction(L, l_input_getMousePosition);
    lua_setfield(L, -2, "getMousePosition");
    lua_pushcfunction(L, l_input_mouseButtonPressed);
    lua_setfield(L, -2, "mouseButtonPressed");
    lua_setfield(L, -2, "input");

    // Register volta.audio
    lua_newtable(L);
    lua_pushcfunction(L, l_audio_loadAudio);
    lua_setfield(L, -2, "loadAudio");
    lua_pushcfunction(L, l_audio_setGlobalVolume);
    lua_setfield(L, -2, "setGlobalVolume");
    lua_pushcfunction(L, l_audio_getGlobalVolume);
    lua_setfield(L, -2, "getGlobalVolume");
    lua_setfield(L, -2, "audio");

    // Register volta.json
    lua_newtable(L);
    lua_pushcfunction(L, l_json_decode);
    lua_setfield(L, -2, "decode");
    lua_pushcfunction(L, l_json_encode);
    lua_setfield(L, -2, "encode");
    lua_setfield(L, -2, "json");

    // Register volta.buffer
    lua_newtable(L);
    lua_pushcfunction(L, l_buffer_alloc);
    lua_setfield(L, -2, "alloc");
    lua_setfield(L, -2, "buffer");

    // Register volta.vector2
    lua_newtable(L);
    lua_pushcfunction(L, l_vector2_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "vector2");

    lua_newtable(L); // volta.particleEmitter
    lua_pushcfunction(L, l_particleEmitter_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "particleEmitter");

    // Register volta.getRunningTime
    lua_pushcfunction(L, l_getRunningTime);
    lua_setfield(L, -2, "getRunningTime");
    lua_pushcfunction(L, l_onCustomEvent);
    lua_setfield(L, -2, "onEvent");
    lua_pushcfunction(L, l_triggerCustomEvent);
    lua_setfield(L, -2, "triggerEvent");

    // Set the table as global 'volta'
    lua_setglobal(L, "volta");

    // Register Buffer metatable
    luaL_newmetatable(L, "Buffer");
    lua_newtable(L); // Index table for methods
    lua_pushcfunction(L, l_buffer_writeUInt8);
    lua_setfield(L, -2, "writeUInt8");
    lua_pushcfunction(L, l_buffer_readUInt8);
    lua_setfield(L, -2, "readUInt8");
    lua_pushcfunction(L, l_buffer_writeInt8);
    lua_setfield(L, -2, "writeInt8");
    lua_pushcfunction(L, l_buffer_readInt8);
    lua_setfield(L, -2, "readInt8");
    lua_pushcfunction(L, l_buffer_writeUInt16);
    lua_setfield(L, -2, "writeUInt16");
    lua_pushcfunction(L, l_buffer_readUInt16);
    lua_setfield(L, -2, "readUInt16");
    lua_pushcfunction(L, l_buffer_writeInt16);
    lua_setfield(L, -2, "writeInt16");
    lua_pushcfunction(L, l_buffer_readInt16);
    lua_setfield(L, -2, "readInt16");
    lua_pushcfunction(L, l_buffer_writeUInt32);
    lua_setfield(L, -2, "writeUInt32");
    lua_pushcfunction(L, l_buffer_readUInt32);
    lua_setfield(L, -2, "readUInt32");
    lua_pushcfunction(L, l_buffer_writeInt32);
    lua_setfield(L, -2, "writeInt32");
    lua_pushcfunction(L, l_buffer_readInt32);
    lua_setfield(L, -2, "readInt32");
    lua_pushcfunction(L, l_buffer_writeUInt64);
    lua_setfield(L, -2, "writeUInt64");
    lua_pushcfunction(L, l_buffer_readUInt64);
    lua_setfield(L, -2, "readUInt64");
    lua_pushcfunction(L, l_buffer_writeInt64);
    lua_setfield(L, -2, "writeInt64");
    lua_pushcfunction(L, l_buffer_readInt64);
    lua_setfield(L, -2, "readInt64");
    lua_pushcfunction(L, l_buffer_size);
    lua_setfield(L, -2, "size");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ParticleEmitter");
    lua_newtable(L);
    lua_pushcfunction(L, l_particleEmitter_emit);
    lua_setfield(L, -2, "emit");
    lua_pushcfunction(L, l_particleEmitter_render);
    lua_setfield(L, -2, "render");
    lua_pushcfunction(L, l_particleEmitter_setLifetime);
    lua_setfield(L, -2, "setLifetime");
    lua_pushcfunction(L, l_particleEmitter_setSpeed);
    lua_setfield(L, -2, "setSpeed");
    lua_pushcfunction(L, l_particleEmitter_setSpread);
    lua_setfield(L, -2, "setSpread");
    lua_pushcfunction(L, l_particleEmitter_getLifetime);
    lua_setfield(L, -2, "getLifetime");
    lua_pushcfunction(L, l_particleEmitter_getSpeed);
    lua_setfield(L, -2, "getSpeed");
    lua_pushcfunction(L, l_particleEmitter_getSpread);
    lua_setfield(L, -2, "getSpread");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    // Extend math table
    lua_getglobal(L, "math");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    lua_pushcfunction(L, l_math_clamp);
    lua_setfield(L, -2, "clamp");
    lua_pushcfunction(L, l_math_round);
    lua_setfield(L, -2, "round");
    lua_pushcfunction(L, l_math_lerp);
    lua_setfield(L, -2, "lerp");
    lua_pushcfunction(L, l_math_tween);
    lua_setfield(L, -2, "tween");
    lua_pushcfunction(L, l_math_noise1d);
    lua_setfield(L, -2, "noise1d");
    lua_pushcfunction(L, l_math_noise2d);
    lua_setfield(L, -2, "noise2d");
    lua_pushcfunction(L, l_math_noise3d);
    lua_setfield(L, -2, "noise3d");
    lua_setglobal(L, "math");

    // Extend table
    lua_getglobal(L, "table");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    lua_pushcfunction(L, l_table_shallowCopy);
    lua_setfield(L, -2, "shallowCopy");
    lua_setglobal(L, "table");

    // Final verification
    luaL_getmetatable(L, "Vector2");
    lua_getfield(L, -1, "__index");
    lua_pop(L, 2);
}

int main() {
    VoltaFramework game {};
    game.run();
    return 0;
}