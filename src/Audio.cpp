#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "VoltaFramework.hpp"
#include <iostream>

int l_audio_loadAudio(lua_State* L) {
    const char* filename {luaL_checkstring(L, 1)};
    VoltaFramework* framework {getFramework(L)};
    if (!framework) {
        lua_pushnil(L);
        return 1;
    }

    ma_sound* sound {framework->loadAudio(filename)};
    if (!sound) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    lua_pushlightuserdata(L, sound);
    lua_setfield(L, -2, "__sound");
    lua_pushcfunction(L, l_audio_play);
    lua_setfield(L, -2, "play");
    lua_pushcfunction(L, l_audio_stop);
    lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, l_audio_setVolume);
    lua_setfield(L, -2, "setVolume");
    lua_pushcfunction(L, l_audio_getVolume);
    lua_setfield(L, -2, "getVolume");
    lua_pushcfunction(L, l_audio_setLooped);
    lua_setfield(L, -2, "setLooped");

    return 1;
}

int l_audio_play(lua_State* L) {
    lua_getfield(L, 1, "__sound");
    ma_sound* sound {static_cast<ma_sound*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);

    VoltaFramework* framework {getFramework(L)};
    if (sound && framework) {
        float currentVolume {ma_sound_get_volume(sound) / framework->getGlobalVolume()}; // Extract original volume
        ma_sound_set_volume(sound, currentVolume * framework->getGlobalVolume()); // Apply global volume
        ma_sound_start(sound);
    } else {
        std::cerr << "Invalid sound or framework object in play!" << std::endl;
    }
    return 0;
}

int l_audio_stop(lua_State* L) {
    lua_getfield(L, 1, "__sound");
    ma_sound* sound {static_cast<ma_sound*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);

    if (sound) {
        ma_sound_stop(sound);
    }
    return 0;
}

int l_audio_setVolume(lua_State* L) {
    lua_Number volume {luaL_checknumber(L, 2)};
    if (volume < 0.0) volume = 0.0;
    if (volume > 1.0) volume = 1.0;

    lua_getfield(L, 1, "__sound");
    ma_sound* sound {static_cast<ma_sound*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);

    VoltaFramework* framework {getFramework(L)};
    if (sound && framework) {
        float effectiveVolume {static_cast<float>(volume * framework->getGlobalVolume())};
        ma_sound_set_volume(sound, effectiveVolume);
    }
    return 0;
}

int l_audio_setLooped(lua_State* L) {
    bool looped {lua_toboolean(L, 2) != 0};

    lua_getfield(L, 1, "__sound");
    ma_sound* sound {static_cast<ma_sound*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);

    if (sound) {
        ma_sound_set_looping(sound, looped);
    }
    return 0;
}

int l_audio_getVolume(lua_State* L) {
    lua_getfield(L, 1, "__sound");
    ma_sound* sound {static_cast<ma_sound*>(lua_touserdata(L, -1))};
    lua_pop(L, 1);

    VoltaFramework* framework {getFramework(L)};
    if (sound && framework) {
        float effectiveVolume {ma_sound_get_volume(sound)};
        float globalVolume {framework->getGlobalVolume()};
        // Return the volume without the global volume applied
        float originalVolume {(globalVolume > 0.0f) ? effectiveVolume / globalVolume : effectiveVolume};
        lua_pushnumber(L, originalVolume);
        return 1;
    }
    lua_pushnumber(L, 0.0f);
    return 1;
}

int l_audio_setGlobalVolume(lua_State* L) {
    lua_Number volume {luaL_checknumber(L, 1)};
    VoltaFramework* framework {getFramework(L)};
    if (framework) {
        framework->setGlobalVolume(static_cast<float>(volume));
    }
    return 0;
}

int l_audio_getGlobalVolume(lua_State* L) {
    VoltaFramework* framework {getFramework(L)};
    if (!framework) {
        std::cerr << "getGlobalVolume: Framework is null\n";
        lua_pushnumber(L, 0.0f);
        return 1;
    }
    lua_pushnumber(L, framework->getGlobalVolume());
    return 1;
}

ma_sound* VoltaFramework::loadAudio(const std::string& filename) {
    if (audioCache.find(filename) != audioCache.end()) {
        return &audioCache[filename];
    }

    std::string fullPath {std::string("assets/") + filename};
    ma_sound* sound {&audioCache[filename]};
    ma_result result {ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, sound)};
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio file: " << fullPath << " (Error: " << result << ")" << std::endl;
        audioCache.erase(filename);
        return nullptr;
    }
    // Apply initial global volume
    ma_sound_set_volume(sound, ma_sound_get_volume(sound) * globalVolume);
    return sound;
}

void VoltaFramework::setGlobalVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    // If current volume is 0, reset individual sound volumes instead of scaling
    if (globalVolume == 0.0f) {
        for (auto& pair : audioCache) {
            ma_sound* sound {&pair.second};
            ma_sound_set_volume(sound, volume);
        }
    } else {
        float scaleFactor {volume / globalVolume};
        for (auto& pair : audioCache) {
            ma_sound* sound {&pair.second};
            float currentVolume {ma_sound_get_volume(sound)};
            ma_sound_set_volume(sound, currentVolume * scaleFactor);
        }
    }
    
    globalVolume = volume;
}