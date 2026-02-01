#pragma once
#include <SFML/Graphics.hpp>

class Map;
class Player;

class Raycaster
{
public:
    Raycaster(int screenWidth, int screenHeight);
    
    void render(sf::RenderWindow& window, const Player& player, const Map& map);
    
private:
    struct RayHit
    {
        float distance;
        bool hitVertical;
        int mapX;
        int mapY;
    };
    
    RayHit castRay(float rayAngle, const Player& player, const Map& map);
    
    int m_screenWidth;
    int m_screenHeight;
    float m_fov; // Field of View (угол обзора)
};
