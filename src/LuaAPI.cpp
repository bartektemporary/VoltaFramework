#include "VoltaFramework.hpp"
#include "Color.hpp"

void VoltaFramework::registerLuaAPI() {
    // Add Camera2D metatable and methods
    luaL_newmetatable(L, "Camera2D");
    lua_newtable(L); // Create table for methods
    lua_pushcfunction(L, l_camera2d_getPosition);
    lua_setfield(L, -2, "getPosition");
    lua_pushcfunction(L, l_camera2d_setPosition);
    lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, l_camera2d_getZoom);
    lua_setfield(L, -2, "getZoom");
    lua_pushcfunction(L, l_camera2d_setZoom);
    lua_setfield(L, -2, "setZoom");
    lua_pushcfunction(L, l_camera2d_getRotation);
    lua_setfield(L, -2, "getRotation");
    lua_pushcfunction(L, l_camera2d_setRotation);
    lua_setfield(L, -2, "setRotation");
    lua_pushcfunction(L, l_camera2d_move);
    lua_setfield(L, -2, "move");
    lua_pushcfunction(L, l_camera2d_zoomBy);
    lua_setfield(L, -2, "zoomBy");
    lua_pushcfunction(L, l_camera2d_rotateBy);
    lua_setfield(L, -2, "rotateBy");
    lua_pushcfunction(L, l_camera2d_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_setfield(L, -2, "__index"); // Set as __index for Camera2D

    lua_pop(L, 1); // Pop the Camera2D metatable

    // Vector2 setup (unchanged)
    luaL_newmetatable(L, "Vector2");
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

    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "Vector2Methods");

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
        lua_remove(L, -2);
        return 1;
    });
    lua_setfield(L, -3, "__index");

    lua_pushcfunction(L, [](lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        luaL_error(L, "Cannot modify Vector2: field '%s' is read-only", key);
        return 0;
    });
    lua_setfield(L, -3, "__newindex");

    lua_pop(L, 1);  // Pop the Vector2 metatable
    // Note: Keeping the extra pop to match Vector2 exactly
    lua_pop(L, 1);

    // Vector3 setup (updated to match Vector2)
    luaL_newmetatable(L, "Vector3");

    lua_pushcfunction(L, l_vector3_add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, l_vector3_subtract);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, l_vector3_multiply);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, l_vector3_divide);
    lua_setfield(L, -2, "__div");
    lua_pushcfunction(L, l_vector3_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_newtable(L);
    lua_pushcfunction(L, l_vector3_add);
    lua_setfield(L, -2, "add");
    lua_pushcfunction(L, l_vector3_subtract);
    lua_setfield(L, -2, "subtract");
    lua_pushcfunction(L, l_vector3_multiply);
    lua_setfield(L, -2, "multiply");
    lua_pushcfunction(L, l_vector3_divide);
    lua_setfield(L, -2, "divide");
    lua_pushcfunction(L, l_vector3_magnitude);
    lua_setfield(L, -2, "magnitude");
    lua_pushcfunction(L, l_vector3_normalize);
    lua_setfield(L, -2, "normalize");
    lua_pushcfunction(L, l_vector3_dot);
    lua_setfield(L, -2, "dot");
    lua_pushcfunction(L, l_vector3_lerp);
    lua_setfield(L, -2, "lerp");
    lua_pushcfunction(L, l_vector3_distance);
    lua_setfield(L, -2, "distance");
    lua_pushcfunction(L, l_vector3_angle);
    lua_setfield(L, -2, "angle");
    lua_pushcfunction(L, l_vector3_tween);
    lua_setfield(L, -2, "tween");

    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "Vector3Methods");

    lua_pushcfunction(L, [](lua_State* L) {
        Vector3* vec = static_cast<Vector3*>(luaL_checkudata(L, 1, "Vector3"));
        const char* key = luaL_checkstring(L, 2);
        if (strcmp(key, "x") == 0) {
            lua_pushnumber(L, vec->x);
            return 1;
        }
        if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, vec->y);
            return 1;
        }
        if (strcmp(key, "z") == 0) {
            lua_pushnumber(L, vec->z);
            return 1;
        }
        lua_getfield(L, LUA_REGISTRYINDEX, "Vector3Methods");
        lua_getfield(L, -1, key);
        lua_remove(L, -2);
        return 1;
    });
    lua_setfield(L, -3, "__index");

    lua_pushcfunction(L, [](lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        luaL_error(L, "Cannot modify Vector3: field '%s' is read-only", key);
        return 0;
    });
    lua_setfield(L, -3, "__newindex");

    lua_pop(L, 1);  // Pop the Vector3 metatable
    lua_pop(L, 1);  // Extra pop to match Vector2 exactly

    luaL_newmetatable(L, "Color");

    // Set __index metamethod with methods
    lua_pushstring(L, "__index");
    lua_newtable(L);  // Create table for methods
    
    // Add instance methods
    lua_pushcfunction(L, l_color_toHex);
    lua_setfield(L, -2, "toHex");
    lua_pushcfunction(L, l_color_toHSV);
    lua_setfield(L, -2, "toHSV");
    lua_pushcfunction(L, l_color_toRGB);
    lua_setfield(L, -2, "toRGB");
    lua_pushcfunction(L, l_color_lerp);
    lua_setfield(L, -2, "lerp");
    lua_pushcfunction(L, l_color_tween);
    lua_setfield(L, -2, "tween");
    
    // Add property getters
    lua_pushcclosure(L, [](lua_State* L) {
        Color* color = checkColor(L, 1);
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "r") == 0) {
            lua_pushnumber(L, color->r);
            return 1;
        }
        if (strcmp(key, "g") == 0) {
            lua_pushnumber(L, color->g);
            return 1;
        }
        if (strcmp(key, "b") == 0) {
            lua_pushnumber(L, color->b);
            return 1;
        }
        
        // Look up methods in the __index table
        lua_getfield(L, lua_upvalueindex(1), key);
        if (!lua_isnil(L, -1)) {
            return 1;
        }
        lua_pop(L, 1);
        
        lua_pushnil(L);
        return 1;
    }, 1);  // 1 upvalue: the method table
    lua_settable(L, -3);

    // Set __newindex metamethod
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, [](lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        luaL_error(L, "Cannot modify Color: field '%s' is read-only", key);
        return 0;
    });
    lua_settable(L, -3);

    // Set __tostring metamethod
    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, l_color_tostring);
    lua_settable(L, -3);

    lua_pop(L, 1);  // Pop the Color metatable

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
    lua_pushcfunction(L, l_drawLine);
    lua_setfield(L, -2, "drawLine");
    lua_pushcfunction(L, l_setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, l_drawImage);
    lua_setfield(L, -2, "drawImage");
    lua_pushcfunction(L, l_setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, l_loadFont);
    lua_setfield(L, -2, "loadFont");
    lua_pushcfunction(L, l_setFont);
    lua_setfield(L, -2, "setFont");
    lua_pushcfunction(L, l_drawText);
    lua_setfield(L, -2, "drawText");
    lua_pushcfunction(L, l_drawCube);
    lua_setfield(L, -2, "drawCube");
    
    lua_pushcfunction(L, l_setFilter);
    lua_setfield(L, -2, "setFilter");
    lua_pushcfunction(L, l_setCustomShader);
    lua_setfield(L, -2, "setCustomShader");
    lua_pushcfunction(L, l_setShader);
    lua_setfield(L, -2, "setShader");
    lua_pushcfunction(L, l_useCustomShader);
    lua_setfield(L, -2, "setUseCustomShader");
    lua_pushcfunction(L, l_clearCustomShader);
    lua_setfield(L, -2, "clearCustomShader");
    lua_pushcfunction(L, l_setCustomShaderUniform);
    lua_setfield(L, -2, "setCustomShaderUniform");
    lua_pushcfunction(L, l_setPositionMode);
    lua_setfield(L, -2, "setPositionMode");
    lua_setfield(L, -2, "graphics");

    lua_newtable(L);
    lua_pushlightuserdata(L, window);
    lua_pushcclosure(L, l_isKeyDown, 1);
    lua_setfield(L, -2, "isKeyDown");
    lua_pushcfunction(L, l_input_keyPressed);
    lua_setfield(L, -2, "keyPressed");
    lua_pushcfunction(L, l_input_getPressedKeys);
    lua_setfield(L, -2, "getPressedKeys");
    lua_pushcfunction(L, l_input_isMouseButtonDown);
    lua_setfield(L, -2, "isMouseButtonDown");
    lua_pushcfunction(L, l_input_getMousePosition);
    lua_setfield(L, -2, "getMousePosition");
    lua_pushcfunction(L, l_input_mouseButtonPressed);
    lua_setfield(L, -2, "mouseButtonPressed");
    lua_pushcfunction(L, l_input_getPressedMouseButtons);
    lua_setfield(L, -2, "getPressedMouseButtons");
    lua_pushcfunction(L, l_input_isGamepadConnected);
    lua_setfield(L, -2, "isGamepadConnected");
    lua_pushcfunction(L, l_input_isGamepadButtonDown);
    lua_setfield(L, -2, "isGamepadButtonDown");
    lua_pushcfunction(L, l_input_gamepadConnected);
    lua_setfield(L, -2, "gamepadConnected");
    lua_pushcfunction(L, l_input_gamepadDisconnected);
    lua_setfield(L, -2, "gamepadDisconnected");
    lua_pushcfunction(L, l_input_getGamepadButtonPressed);
    lua_setfield(L, -2, "getGamepadButtonPressed");
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
    lua_pushcfunction(L, l_filesystem_exists);
    lua_setfield(L, -2, "exists");
    lua_pushcfunction(L, l_filesystem_isDirectory);
    lua_setfield(L, -2, "isDirectory");
    lua_pushcfunction(L, l_filesystem_createDirectory);
    lua_setfield(L, -2, "createDirectory");
    lua_pushcfunction(L, l_filesystem_remove);
    lua_setfield(L, -2, "remove");
    lua_pushcfunction(L, l_filesystem_listDir);
    lua_setfield(L, -2, "listDirectory");
    lua_pushcfunction(L, l_filesystem_getWorkingDir);
    lua_setfield(L, -2, "getWorkingDirectory");
    lua_pushcfunction(L, l_filesystem_setWorkingDir);
    lua_setfield(L, -2, "setWorkingDirectory");
    lua_pushcfunction(L, l_filesystem_getFileSize);
    lua_setfield(L, -2, "getFileSize");
    lua_pushcfunction(L, l_filesystem_isRegularFile);
    lua_setfield(L, -2, "isRegularFile");
    lua_pushcfunction(L, l_filesystem_isCharacterFile);
    lua_setfield(L, -2, "isCharacterFile");
    lua_pushcfunction(L, l_filesystem_isFifo);
    lua_setfield(L, -2, "isFifo");
    lua_pushcfunction(L, l_filesystem_isSocket);
    lua_setfield(L, -2, "isSocket");
    lua_pushcfunction(L, l_filesystem_canonical);
    lua_setfield(L, -2, "canonical");
    lua_pushcfunction(L, l_filesystem_relative);
    lua_setfield(L, -2, "relative");
    lua_pushcfunction(L, l_filesystem_copy);
    lua_setfield(L, -2, "copy");
    lua_pushcfunction(L, l_filesystem_rename);
    lua_setfield(L, -2, "rename");
    lua_pushcfunction(L, l_filesystem_lastWriteTime);
    lua_setfield(L, -2, "lastWriteTime");
    lua_pushcfunction(L, l_filesystem_isEmpty);
    lua_setfield(L, -2, "isEmpty");
    lua_pushcfunction(L, l_filesystem_absolute);
    lua_setfield(L, -2, "absolute");
    lua_pushcfunction(L, l_filesystem_parentPath);
    lua_setfield(L, -2, "parentPath");
    lua_pushcfunction(L, l_filesystem_filename);
    lua_setfield(L, -2, "filename");
    lua_pushcfunction(L, l_filesystem_stem);
    lua_setfield(L, -2, "stem");
    lua_pushcfunction(L, l_filesystem_extension);
    lua_setfield(L, -2, "extension");
    lua_pushcfunction(L, l_filesystem_tempDirectory);
    lua_setfield(L, -2, "tempDirectory");
    lua_pushcfunction(L, l_filesystem_equivalent);
    lua_setfield(L, -2, "equivalent");
    lua_pushcfunction(L, l_filesystem_getUserDir);
    lua_setfield(L, -2, "getUserDirectory");
    lua_setfield(L, -2, "filesystem");

    lua_newtable(L);
    lua_pushcfunction(L, l_json_decode);
    lua_setfield(L, -2, "decode");
    lua_pushcfunction(L, l_json_encode);
    lua_setfield(L, -2, "encode");
    lua_setfield(L, -2, "json");

    lua_newtable(L);
    
    lua_pushcfunction(L, l_sqlite_open);
    lua_setfield(L, -2, "open");
    lua_pushcfunction(L, l_sqlite_close);
    lua_setfield(L, -2, "close");
    lua_pushcfunction(L, l_sqlite_exec);
    lua_setfield(L, -2, "exec");
    lua_pushcfunction(L, l_sqlite_prepare);
    lua_setfield(L, -2, "prepare");
    lua_pushcfunction(L, l_sqlite_step);
    lua_setfield(L, -2, "step");
    lua_pushcfunction(L, l_sqlite_finalize);
    lua_setfield(L, -2, "finalize");
    lua_pushcfunction(L, l_sqlite_bind);
    lua_setfield(L, -2, "bind");
    lua_pushcfunction(L, l_sqlite_column);
    lua_setfield(L, -2, "column");
    lua_setfield(L, -2, "sqlite");

    lua_newtable(L);
    lua_pushcfunction(L, l_buffer_alloc);
    lua_setfield(L, -2, "alloc");
    lua_setfield(L, -2, "buffer");

    lua_newtable(L);
    lua_pushcfunction(L, l_color_create);
    lua_setfield(L, -2, "new");
    lua_pushcfunction(L, l_color_fromRGB);
    lua_setfield(L, -2, "fromRGB");
    lua_pushcfunction(L, l_color_newHSV);
    lua_setfield(L, -2, "fromHSV");
    lua_pushcfunction(L, l_color_newHex);
    lua_setfield(L, -2, "fromHex");
    lua_setfield(L, -2, "color");

    lua_newtable(L);
    lua_pushcfunction(L, l_vector2_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "vector2");

    lua_newtable(L);
    lua_pushcfunction(L, l_vector3_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "vector3");

    lua_newtable(L);
    lua_pushcfunction(L, l_camera2d_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "camera2d");

    lua_newtable(L);
    lua_pushcfunction(L, l_particleEmitter_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "particleEmitter");

    lua_pushcfunction(L, l_getRunningTime);
    lua_setfield(L, -2, "getRunningTime");
    lua_pushcfunction(L, l_onCustomEvent);
    lua_setfield(L, -2, "onEvent");
    lua_pushcfunction(L, l_triggerCustomEvent);
    lua_setfield(L, -2, "triggerEvent");
    lua_pushcfunction(L, l_setCamera2D);
    lua_setfield(L, -2, "setCamera2D");

    lua_setglobal(L, "volta");

    luaL_newmetatable(L, "Buffer");
    lua_newtable(L);
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
    lua_pushcfunction(L, l_particleEmitter_setShape);
    lua_setfield(L, -2, "setShape");
    lua_pushcfunction(L, l_particleEmitter_getShape);
    lua_setfield(L, -2, "getShape");
    lua_pushcfunction(L, l_particleEmitter_setSize);
    lua_setfield(L, -2, "setSize");
    lua_pushcfunction(L, l_particleEmitter_getSize);
    lua_setfield(L, -2, "getSize");
    lua_pushcfunction(L, l_particleEmitter_setDirection);
    lua_setfield(L, -2, "setDirection");
    lua_pushcfunction(L, l_particleEmitter_getDirection);
    lua_setfield(L, -2, "getDirection");
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
    lua_pushcfunction(L, l_particleEmitter_setPosition);
    lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, l_particleEmitter_getPosition);
    lua_setfield(L, -2, "getPosition");
    lua_pushcfunction(L, l_particleEmitter_setTexture);
    lua_setfield(L, -2, "setTexture");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

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

    lua_getglobal(L, "table");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    lua_pushcfunction(L, l_table_shallowCopy);
    lua_setfield(L, -2, "shallowCopy");
    lua_setglobal(L, "table");

    luaL_getmetatable(L, "Vector2");
    lua_getfield(L, -1, "__index");
    lua_pop(L, 2);
}