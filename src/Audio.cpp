#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "Audio.hpp"
#include "VoltaFramework.hpp"
#include <iostream>

// Audio class implementation
Audio::Audio(ma_engine* engine, const std::string& filename, float globalVolume) 
    : initialized(false), originalVolume(1.0f) {
    ma_result result = ma_sound_init_from_file(engine, filename.c_str(), 0, NULL, NULL, &sound);
    if (result == MA_SUCCESS) {
        initialized = true;
        applyGlobalVolume(globalVolume);
    } else {
        std::cerr << "Failed to load audio file: " << filename << " (Error: " << result << ")" << std::endl;
    }
}

Audio::Audio(Audio&& other) noexcept 
    : sound(other.sound), initialized(other.initialized), originalVolume(other.originalVolume) {
    // Zero out the moved-from object's sound to prevent double uninit
    other.initialized = false;
    // No need to uninit other.sound here; it’s safe as long as initialized is false
}

Audio::~Audio() {
    if (initialized) {
        ma_sound_uninit(&sound);
    }
}

void Audio::play() {
    if (initialized) {
        ma_sound_start(&sound);
    }
}

void Audio::stop() {
    if (initialized) {
        ma_sound_stop(&sound);
    }
}

bool Audio::isPlaying() const {
    return initialized && ma_sound_is_playing(&sound);
}

void Audio::setVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    originalVolume = volume;
    if (initialized) {
        ma_sound_set_volume(&sound, volume); // Global volume applied separately
    }
}

float Audio::getVolume() const {
    return originalVolume;
}

void Audio::setLooped(bool looped) {
    if (initialized) {
        ma_sound_set_looping(&sound, looped);
    }
}

bool Audio::isLooped() const {
    return initialized && ma_sound_is_looping(&sound);
}

void Audio::applyGlobalVolume(float globalVolume) {
    if (initialized) {
        ma_sound_set_volume(&sound, originalVolume * globalVolume);
    }
}

// Lua bindings helper
static Audio* checkAudio(lua_State* L, int index) {
    lua_getfield(L, index, "__audio");
    Audio* audio = static_cast<Audio*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (!audio) luaL_error(L, "Invalid audio object");
    return audio;
}

// Lua bindings (unchanged)
int l_audio_loadAudio(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        lua_pushnil(L);
        return 1;
    }

    Audio* audio = framework->loadAudio(filename);
    if (!audio) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    lua_pushlightuserdata(L, audio);
    lua_setfield(L, -2, "__audio");

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
    Audio* audio = checkAudio(L, 1);
    audio->play();
    return 0;
}

int l_audio_stop(lua_State* L) {
    Audio* audio = checkAudio(L, 1);
    audio->stop();
    return 0;
}

int l_audio_setVolume(lua_State* L) {
    Audio* audio = checkAudio(L, 1);
    float volume = static_cast<float>(luaL_checknumber(L, 2));
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    VoltaFramework* framework = getFramework(L);
    if (framework) {
        audio->setVolume(volume);
        audio->applyGlobalVolume(framework->getGlobalVolume());
    }
    return 0;
}

int l_audio_getVolume(lua_State* L) {
    Audio* audio = checkAudio(L, 1);
    lua_pushnumber(L, audio->getVolume());
    return 1;
}

int l_audio_setLooped(lua_State* L) {
    Audio* audio = checkAudio(L, 1);
    bool looped = lua_toboolean(L, 2) != 0;
    audio->setLooped(looped);
    return 0;
}

int l_audio_setGlobalVolume(lua_State* L) {
    float volume = static_cast<float>(luaL_checknumber(L, 1));
    VoltaFramework* framework = getFramework(L);
    if (framework) {
        framework->setGlobalVolume(volume);
    }
    return 0;
}

int l_audio_getGlobalVolume(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        std::cerr << "getGlobalVolume: Framework is null\n";
        lua_pushnumber(L, 0.0f);
        return 1;
    }
    lua_pushnumber(L, framework->getGlobalVolume());
    return 1;
}

// Updated VoltaFramework::loadAudio
Audio* VoltaFramework::loadAudio(const std::string& filename) {
    auto it = audioCache.find(filename);
    if (it != audioCache.end()) {
        return it->second.get();
    }

    std::string fullPath = loadFile(filename);
    if (fullPath.empty()) {
        return nullptr;
    }

    auto audio = std::make_unique<Audio>(&engine, fullPath, globalVolume);
    if (!audio->initialized) {
        return nullptr;
    }

    auto [inserted_it, success] = audioCache.emplace(filename, std::move(audio));
    return inserted_it->second.get();
}

void VoltaFramework::setGlobalVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (globalVolume == 0.0f) {
        for (auto& pair : audioCache) {
            pair.second->setVolume(pair.second->getVolume()); // Reset original volume
            pair.second->applyGlobalVolume(volume);
        }
    } else {
        for (auto& pair : audioCache) {
            pair.second->applyGlobalVolume(volume);
        }
    }

    globalVolume = volume;
}