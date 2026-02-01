#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

class Player;
class Map;

struct Light
{
    float x, y;           // Позиция источника света
    float radius;         // Радиус освещения
    float intensity;      // Интенсивность (0.0 - 1.0)
    sf::Color color;      // Цвет света
    bool isStatic;        // Статичный (комната) или динамический (фонарик)
    
    Light(float px, float py, float r, float i = 1.0f, sf::Color c = sf::Color::White, bool stat = false)
        : x(px), y(py), radius(r), intensity(i), color(c), isStatic(stat) {}
};

class LightSystem
{
public:
    LightSystem();
    
    // Вычисляет освещенность точки от всех источников света
    float calculateLighting(float x, float y, const Player& player, const Map& map) const;
    
    // Добавляет статичные источники света (комнаты)
    void addRoomLights(const Map& map);
    
    // Очищает все источники света
    void clearLights();
    
    // Управление фонариком
    void setFlashlightEnabled(bool enabled) { m_flashlightEnabled = enabled; }
    bool isFlashlightEnabled() const { return m_flashlightEnabled; }
    
    void setFlashlightBattery(float battery) { m_flashlightBattery = battery; }
    float getFlashlightBattery() const { return m_flashlightBattery; }
    
    // Подзарядка батареи (при подборе предмета)
    void rechargeBattery(float amount) 
    { 
        m_flashlightBattery += amount;
        if (m_flashlightBattery > 100.0f)
            m_flashlightBattery = 100.0f;
    }
    
    void updateFlashlight(float deltaTime, bool isUsing, bool inSafeRoom = false);
    
    // Параметры фонарика
    float getFlashlightRadius() const { return m_flashlightRadius; }
    float getFlashlightAngle() const { return m_flashlightAngle; }
    
private:
    std::vector<Light> m_staticLights;  // Статичные источники (комнаты)
    
    // Параметры фонарика
    bool m_flashlightEnabled;
    float m_flashlightBattery;          // 0.0 - 100.0
    float m_flashlightRadius;           // Радиус луча
    float m_flashlightAngle;            // Угол конуса (в радианах)
    float m_flashlightDrainRate;        // Скорость разряда батареи
    
    // Ambient освещение
    float m_ambientLight;               // Минимальное освещение (очень темное)
    
    // Проверяет, есть ли прямая видимость между двумя точками
    bool hasLineOfSight(float x1, float y1, float x2, float y2, const Map& map) const;
};
