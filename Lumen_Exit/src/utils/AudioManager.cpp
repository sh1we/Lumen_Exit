#include "AudioManager.h"
#include "EmbeddedSounds.h"
#include <iostream>

AudioManager::AudioManager()
{
    // load embedded sounds
    loadSoundFromMemory("scroll", scroll_data, scroll_size);
    loadSoundFromMemory("click", interaction_data, interaction_size);
}

bool AudioManager::loadSoundFromMemory(const std::string& name, const unsigned char* data, size_t size)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromMemory(data, size))
    {
        std::cerr << "Failed to load sound from memory: " << name << std::endl;
        return false;
    }
    
    m_soundBuffers[name] = std::move(buffer);
    std::cout << "Sound loaded from memory: " << name << std::endl;
    return true;
}

bool AudioManager::loadSound(const std::string& name, const std::string& filepath)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filepath))
    {
        std::cerr << "Failed to load sound: " << filepath << std::endl;
        return false;
    }
    
    m_soundBuffers[name] = std::move(buffer);
    std::cout << "Sound loaded: " << name << std::endl;
    return true;
}

bool AudioManager::loadMusic(const std::string& name, const std::string& filepath)
{
    // music streams from file, just store path
    m_currentMusicPath = filepath;
    std::cout << "Music path set: " << filepath << std::endl;
    return true;
}

void AudioManager::playSound(const std::string& name, float volume)
{
    cleanupFinishedSounds();
    
    auto it = m_soundBuffers.find(name);
    if (it == m_soundBuffers.end())
    {
        std::cerr << "Sound not found: " << name << std::endl;
        return;
    }
    
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(it->second);
    // apply sfx and master volume
    sound->setVolume(volume * m_sfxVolume / 100.0f * m_masterVolume / 100.0f);
    sound->play();
    
    m_activeSounds.push_back(std::move(sound));
}

void AudioManager::playMusic(const std::string& name, float volume, bool loop)
{
    // use embedded ambient data
    if (name == "ambient")
    {
        m_musicData.assign(ambient_data, ambient_data + ambient_size);
        
        if (!m_music.openFromMemory(m_musicData.data(), m_musicData.size()))
        {
            std::cerr << "Failed to load music from memory: " << name << std::endl;
            return;
        }
    }
    else
    {
        std::string filepath = "assets/sounds/" + name + ".ogg";
        if (!m_music.openFromFile(filepath))
        {
            std::cerr << "Failed to load music: " << filepath << std::endl;
            return;
        }
    }
    
    m_music.setVolume(volume * m_masterVolume / 100.0f);
    m_music.setLoop(loop);
    m_music.play();
    
    std::cout << "Playing music: " << name << std::endl;
}

void AudioManager::stopMusic()
{
    m_music.stop();
}

void AudioManager::pauseMusic()
{
    m_music.pause();
}

void AudioManager::resumeMusic()
{
    m_music.play();
}

void AudioManager::setMusicVolume(float volume)
{
    m_music.setVolume(volume * m_masterVolume / 100.0f);
}

void AudioManager::setMasterVolume(float volume)
{
    m_masterVolume = volume;
    // update music volume with new master
    m_music.setVolume(m_musicVolume * m_masterVolume / 100.0f);
}

void AudioManager::setMusicVolumeLevel(float volume)
{
    m_musicVolume = volume;
    m_music.setVolume(m_musicVolume * m_masterVolume / 100.0f);
}

void AudioManager::cleanupFinishedSounds()
{
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& sound) {
                return sound->getStatus() == sf::Sound::Stopped;
            }),
        m_activeSounds.end()
    );
}
