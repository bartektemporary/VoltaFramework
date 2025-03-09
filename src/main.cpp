#include "VoltaFramework.hpp"

void VoltaFramework::registerLuaAPI() {
    lua_newtable(L);

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

    lua_newtable(L);
    lua_pushcfunction(L, l_rectangle);
    lua_setfield(L, -2, "rectangle");
    lua_pushcfunction(L, l_circle);
    lua_setfield(L, -2, "circle");
    lua_pushcfunction(L, l_setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, l_drawImage);
    lua_setfield(L, -2, "drawImage");
    lua_pushcfunction(L, l_setFilter);
    lua_setfield(L, -2, "setFilter");
    lua_setfield(L, -2, "graphics");

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

    lua_newtable(L);
    lua_pushcfunction(L, l_audio_loadAudio);
    lua_setfield(L, -2, "loadAudio");
    lua_pushcfunction(L, l_audio_setGlobalVolume);
    lua_setfield(L, -2, "setGlobalVolume");
    lua_pushcfunction(L, l_audio_getGlobalVolume);
    lua_setfield(L, -2, "getGlobalVolume");
    lua_setfield(L, -2, "audio");

    lua_newtable(L);
    lua_pushcfunction(L, l_json_decode);
    lua_setfield(L, -2, "decode");
    lua_pushcfunction(L, l_json_encode);
    lua_setfield(L, -2, "encode");
    lua_setfield(L, -2, "json");

    lua_newtable(L);
    lua_pushcfunction(L, l_buffer_alloc);
    lua_setfield(L, -2, "alloc");
    lua_setfield(L, -2, "buffer");

    lua_setglobal(L, "volta");

    // Register buffer metatable
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
    lua_pop(L, 1); // Pop metatable

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
    lua_pushcfunction(L, l_math_noise1d);
    lua_setfield(L, -2, "noise1d");
    lua_pushcfunction(L, l_math_noise2d);
    lua_setfield(L, -2, "noise2d");
    lua_pushcfunction(L, l_math_noise3d);
    lua_setfield(L, -2, "noise3d");
    lua_setglobal(L, "math");

    // Add table extensions
    lua_getglobal(L, "table");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    lua_pushcfunction(L, l_table_shallowCopy);
    lua_setfield(L, -2, "shallowCopy");
    lua_setglobal(L, "table");
}

int main() {
    VoltaFramework game {};
    game.run();
    return 0;
}