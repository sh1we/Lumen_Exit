#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class LoadingScreen
{
public:
    LoadingScreen(float width, float height);
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    bool isFinished() const { return m_finished; }
    
private:
    sf::Font m_font;
    std::vector<std::string> m_lines;
    float m_timer;
    int m_currentLine;
    bool m_finished;
    float m_width;
    float m_height;
};
