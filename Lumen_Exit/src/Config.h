#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct GameConfig
{
    // Видео настройки
    int screenWidth = 1280;
    int screenHeight = 720;
    int targetFPS = 60;
    bool fullscreen = false;
    
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
