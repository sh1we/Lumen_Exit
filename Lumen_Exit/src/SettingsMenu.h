#pragma once
#include <SFML/Graphics.hpp>

class SettingsMenu
{
public:
    SettingsMenu(float width, float height, float& sensitivity);
    
    void draw(sf::RenderWindow& window);
    void handleInput(sf::Keyboard::Key key);
    void handleMouseClick(const sf::Vector2i& mousePos);
    void handleMouseMove(const sf::Vector2i& mousePos, bool isPressed);
    
private:
    sf::Font m_font;
    float m_width;
    float m_height;
    float& m_sensitivity; // Ссылка на чувствительность мыши
    
    // Параметры слайдера
    float m_barX;
    float m_barY;
    float m_barWidth;
    float m_barHeight;
    
    void updateSensitivityFromMouse(float mouseX);
};
