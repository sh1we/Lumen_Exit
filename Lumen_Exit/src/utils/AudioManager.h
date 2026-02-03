#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

class AudioManager
{
public:
    static AudioManager& getInstance()
    {
        static AudioManager instance;
        return instance;
    }
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    // load sounds from memory
    bool loadSoundFromMemory(const std::string& name, const unsigned char* data, size_t size);
    bool loadSound(const std::string& name, const std::string& filepath);
    bool loadMusic(const std::string& name, const std::string& filepath);
    
    // play sounds (one-shot)
    void playSound(const std::string& name, float volume = 100.0f);
    
    // music control (looping background)
    void playMusic(const std::string& name, float volume = 50.0f, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    
    // volume controls
    void setMasterVolume(float volume);
    void setSfxVolume(float volume) { m_sfxVolume = volume; }
    void setMusicVolumeLevel(float volume);
    
    float getMasterVolume() const { return m_masterVolume; }
    float getSfxVolume() const { return m_sfxVolume; }
    float getMusicVolumeLevel() const { return m_musicVolume; }
    
private:
    AudioManager();
    ~AudioManager() = default;
    
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;  // pool for playing sounds
    
    sf::Music m_music;
    std::string m_currentMusicPath;
    
    // embedded music data
    std::vector<unsigned char> m_musicData;
    
    float m_masterVolume = 100.0f;
    float m_musicVolume = 50.0f;
    float m_sfxVolume = 80.0f;
    
    void cleanupFinishedSounds();
};
