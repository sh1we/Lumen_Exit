#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

enum class LightingQuality
{
    LOW = 0,    // 2 samples + interpolation
    MEDIUM = 1, // 3 samples
    HIGH = 2    // 5 samples (adaptive)
};

struct GameConfig
{
    // video
    int screenWidth = 1280;
    int screenHeight = 720;
    int targetFPS = 60;
    bool fullscreen = false;
    
    // graphics
    LightingQuality lightingQuality = LightingQuality::HIGH;
    
    // controls
    float mouseSensitivity = 0.001f;
    
    // gameplay
    unsigned int customSeed = 0;  // 0 = random seed
    
    // stats
    float bestTime = 999999.0f;
    
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);
    
    void updateBestTime(float time);
};
