#pragma once
#include <SFML/Graphics.hpp>

class Map;
class Player;
class LightSystem;
enum class LightingQuality;

class Raycaster
{
public:
    Raycaster(int screenWidth, int screenHeight);
    
    void render(sf::RenderWindow& window, const Player& player, const Map& map, const LightSystem& lightSystem);
    
    // Установка качества освещения
    void setLightingQuality(LightingQuality quality) { m_lightingQuality = quality; }
    
private:
    struct RayHit
    {
        float distance;
        bool hitVertical;
        int mapX;
        int mapY;
        float hitX;  // Точная позиция попадания X
        float hitY;  // Точная позиция попадания Y
    };
    
    RayHit castRay(float rayAngle, const Player& player, const Map& map);
    
    int m_screenWidth;
    int m_screenHeight;
    float m_fov; // Field of View (угол обзора)
    
    // Оптимизация: переиспользуемые буферы
    sf::VertexArray m_floorCeiling;
    sf::VertexArray m_wallSlices;
    
    // Настройки качества
    LightingQuality m_lightingQuality;
    
    // Кэш для интерполяции освещения
    float m_lastLighting;
};
