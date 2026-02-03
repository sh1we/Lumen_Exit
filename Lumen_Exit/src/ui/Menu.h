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
    
    // Переключение между режимами меню (с игрой / без игры)
    void setInGameMode(bool inGame);
    bool isInGameMode() const { return m_inGameMode; }

private:
    void rebuildMenu();
    
    int m_selectedItemIndex;
    std::vector<sf::Text> m_menuItems;
    sf::Font m_font;
    float m_width;
    float m_height;
    bool m_inGameMode; // true если игра уже запущена
};
