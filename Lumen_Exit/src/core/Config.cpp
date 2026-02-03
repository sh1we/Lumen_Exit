#include "Config.h"

bool GameConfig::saveToFile(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to save config to " << filename << std::endl;
        return false;
    }
    
    file << "# Lumen_Exit() Configuration File\n";
    file << "screenWidth=" << screenWidth << "\n";
    file << "screenHeight=" << screenHeight << "\n";
    file << "targetFPS=" << targetFPS << "\n";
    file << "fullscreen=" << (fullscreen ? 1 : 0) << "\n";
    file << "lightingQuality=" << static_cast<int>(lightingQuality) << "\n";
    file << "masterVolume=" << masterVolume << "\n";
    file << "musicVolume=" << musicVolume << "\n";
    file << "sfxVolume=" << sfxVolume << "\n";
    file << "mouseSensitivity=" << mouseSensitivity << "\n";
    file << "customSeed=" << customSeed << "\n";
    file << "bestTime=" << bestTime << "\n";
    
    file.close();
    std::cout << "Config saved to " << filename << std::endl;
    return true;
}

bool GameConfig::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Config file not found, using defaults" << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // apply settings
        if (key == "screenWidth")
            screenWidth = std::stoi(value);
        else if (key == "screenHeight")
            screenHeight = std::stoi(value);
        else if (key == "targetFPS")
            targetFPS = std::stoi(value);
        else if (key == "fullscreen")
            fullscreen = (std::stoi(value) != 0);
        else if (key == "lightingQuality")
        {
            int quality = std::stoi(value);
            if (quality >= 0 && quality <= 2)
                lightingQuality = static_cast<LightingQuality>(quality);
        }
        else if (key == "mouseSensitivity")
            mouseSensitivity = std::stof(value);
        else if (key == "masterVolume")
            masterVolume = std::stof(value);
        else if (key == "musicVolume")
            musicVolume = std::stof(value);
        else if (key == "sfxVolume")
            sfxVolume = std::stof(value);
        else if (key == "customSeed")
            customSeed = static_cast<unsigned int>(std::stoul(value));
        else if (key == "bestTime")
            bestTime = std::stof(value);
    }
    
    file.close();
    std::cout << "Config loaded from " << filename << std::endl;
    return true;
}

void GameConfig::updateBestTime(float time)
{
    if (time < bestTime)
    {
        bestTime = time;
        std::cout << "New best time: " << static_cast<int>(bestTime) << " seconds!" << std::endl;
    }
}
