#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "miniaudio.h"
#include <string>

class VoltaFramework; // Forward declaration

class Audio {
public:
    Audio(ma_engine* engine, const std::string& filename, float globalVolume);
    ~Audio();

    // Move constructor
    Audio(Audio&& other) noexcept;
    Audio& operator=(Audio&& other) noexcept = delete; // Optional: could implement if needed

    // Prevent copying
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void play();
    void stop();
    bool isPlaying() const;

    void setVolume(float volume);
    float getVolume() const;

    void setLooped(bool looped);
    bool isLooped() const;

    void applyGlobalVolume(float globalVolume);

    bool initialized; // Already public in your code

private:
    ma_sound sound;
    float originalVolume;
};

#endif // AUDIO_HPP