#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Menu
{
public:
    Menu(float width, float height);
    
    void draw(sf::RenderWindow& window);
    void moveUp();
    void moveDown();
    void handleMouseMove(const sf::Vector2i& mousePos);
    bool handleMouseClick(const sf::Vector2i& mousePos);
    int getSelectedItem() const { return m_selectedItemIndex; }

private:
    int m_selectedItemIndex;
    std::vector<sf::Text> m_menuItems;
    sf::Font m_font;
};
