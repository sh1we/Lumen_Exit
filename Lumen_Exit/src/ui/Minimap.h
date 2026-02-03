#pragma once
#include <SFML/Graphics.hpp>

class Map;
class Player;

class Minimap
{
public:
    Minimap(int screenWidth, int screenHeight);
    
    void draw(sf::RenderWindow& window, const Player& player, const Map& map);
    
private:
    int m_screenWidth;
    int m_screenHeight;
    float m_scale; // Масштаб миникарты
};
