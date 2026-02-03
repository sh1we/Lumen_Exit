#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

class Player;
class Map;

struct Light
{
    float x, y;
    float radius;
    float intensity;
    sf::Color color;
    bool isStatic;
    
    Light(float px, float py, float r, float i = 1.0f, sf::Color c = sf::Color::White, bool stat = false)
        : x(px), y(py), radius(r), intensity(i), color(c), isStatic(stat) {}
};

class LightSystem
{
public:
    LightSystem();
    
    float calculateLighting(float x, float y, const Player& player, const Map& map) const;
    void addRoomLights(const Map& map);
    void clearLights();
    
    void setFlashlightEnabled(bool enabled) { m_flashlightEnabled = enabled; }
    bool isFlashlightEnabled() const { return m_flashlightEnabled; }
    
    void setFlashlightBattery(float battery) { m_flashlightBattery = battery; }
    float getFlashlightBattery() const { return m_flashlightBattery; }
    
    void rechargeBattery(float amount) 
    { 
        m_flashlightBattery += amount;
        if (m_flashlightBattery > 100.0f)
            m_flashlightBattery = 100.0f;
    }
    
    void updateFlashlight(float deltaTime, bool isUsing, bool inSafeRoom = false);
    
    float getFlashlightRadius() const { return m_flashlightRadius; }
    float getFlashlightAngle() const { return m_flashlightAngle; }
    
private:
    std::vector<Light> m_staticLights;
    
    bool m_flashlightEnabled;
    float m_flashlightBattery;
    float m_flashlightRadius;
    float m_flashlightAngle;
    float m_flashlightDrainRate;
    
    float m_ambientLight;
    
    bool hasLineOfSight(float x1, float y1, float x2, float y2, const Map& map) const;
};
