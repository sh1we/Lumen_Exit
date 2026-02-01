#pragma once
#include <SFML/Graphics.hpp>

class Player;
class LightSystem;

class HUD
{
public:
    HUD(int screenWidth, int screenHeight);
    
    void draw(sf::RenderWindow& window, const Player& player, float gameTime, const LightSystem& lightSystem);
    
private:
    sf::Font m_font;
    int m_screenWidth;
    int m_screenHeight;
};
