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
    // Видео настройки
    int screenWidth = 1280;
    int screenHeight = 720;
    int targetFPS = 60;
    bool fullscreen = false;
    
    // Графика
    LightingQuality lightingQuality = LightingQuality::HIGH;
    
    // Управление
    float mouseSensitivity = 0.001f;
    
    // Статистика
    float bestTime = 999999.0f; // Лучшее время (в секундах)
    
    // Сохранение/загрузка
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);
    
    // Обновление лучшего времени
    void updateBestTime(float time);
};
